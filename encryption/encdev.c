#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/slab.h>
#include <linux/random.h>

#define DEVICE_NAME "encdev"
#define CLASS_NAME "enc"
#define BUFFER_SIZE 256

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("encdev driver demo");

static char message[BUFFER_SIZE] = {0};
static char encrypted[BUFFER_SIZE][BUFFER_SIZE] = {0, 0};
static int count = 0;
static int readCount = 0;
char randomKey[16];
static struct class* encdevClass = NULL;
static struct device* encdevDevice = NULL;

static int dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int dev_close(struct inode *, struct file *);

static struct file_operations fops =
{
        .read = dev_read,
        .open = dev_open,
        .write = dev_write,
        .release = dev_close,
};

static int __init encdev_init(void)
{
        int t = register_chrdev(89, DEVICE_NAME, &fops);
        if (t < 0)
        {
                printk(KERN_INFO "encdev: Device registration failed.\n");
                return t;
        }
        printk(KERN_INFO "encdev: Device registered successfully.\n");

        encdevClass = class_create(THIS_MODULE, CLASS_NAME);
        if (IS_ERR(encdevClass))
        {
                unregister_chrdev(89, DEVICE_NAME);
                printk(KERN_INFO "encdev: Failed to register device class.\n");
                return PTR_ERR(encdevClass);
        }
        printk(KERN_INFO "encdev: Device class registered successfully.\n");

        encdevDevice = device_create(encdevClass, NULL, MKDEV(89, 0), NULL, DEVICE_NAME);
        if (IS_ERR(encdevDevice))
        {
                class_destroy(encdevClass);
                unregister_chrdev(89, DEVICE_NAME);
                printk(KERN_INFO "encdev: Failed to create the device.\n");
                return PTR_ERR(encdevDevice);
        }
        printk(KERN_INFO "encdev: Device class created successfully.\n");
        return 0;
}

static void __exit encdev_exit(void)
{
        device_destroy(encdevClass, MKDEV(89, 0));
        class_unregister(encdevClass);
        class_destroy(encdevClass);
        unregister_chrdev(89, DEVICE_NAME);
        printk(KERN_INFO "encdev: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inod, struct file *fil)
{
        printk(KERN_INFO "encdev: Device opened.\n");
        return 0;
}

static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
        int times = 0, readPos = 0;
        printk("Count: %d\n", count);
        int length = len;
        if (readCount == 0)
        {
                printk("Key: %s\n", randomKey);
                while (length && randomKey[readPos]!='\0')
                {
                        put_user(randomKey[readPos], buff++);
                        length--;
                        times++;
                        readPos++;
                }
                readCount = readCount + 1;
                return times;
        }
        int track = readCount - 1;
        printk("%d, Enc msg[%d]: %s\n", *off, track, encrypted[track]);
        while (length && encrypted[track][readPos]!='\0')
        {
                put_user(encrypted[track][readPos], buff++);
                length--;
                times++;
                readPos++;
        }
        readCount = readCount + 1;
        return times;
}

static ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
        int nb_bytes_to_copy;
        char res[len];
        int i, length, track;
        if (count == 0)
        {
                track = count;
                struct file *f;
                mm_segment_t fs;
                int j;
                for (j = 0; j < 16; j++)
                        randomKey[j] = 0;
                f = filp_open("/dev/urandom", O_RDONLY, 0);
                if (f == NULL)
                        printk(KERN_INFO "filp_open error!!!\n");
                else
                {
                        fs = get_fs();
                        set_fs(get_ds());
                        f->f_op->read(f, randomKey, 16, &f->f_pos);
                        set_fs(fs);
                }
                filp_close(f, NULL);
                printk(KERN_INFO "Key: %s\n", randomKey);
        }
        else
        {
                track = count - 1;
        }
        if (*off > BUFFER_SIZE)
        {
                return -ENOBUFS;
        }
        if ((BUFFER_SIZE - 1 - *off) < len)
                nb_bytes_to_copy = BUFFER_SIZE - 1 - *off;
        else
                nb_bytes_to_copy = len;
        copy_from_user(message + *off, buff, nb_bytes_to_copy);
        message[*off + nb_bytes_to_copy] = '\0';
        length =  nb_bytes_to_copy;
        if (count ==  0)
        {
                for (i = 0; i < length; i++)
                {
                        res[i] = (char)(randomKey[i] ^ message[i + *off]);
                }
        }
        else
        {
                for (i = 0; i < length; i++)
                {
                        res[i] = (char)(encrypted[track][i] ^ message[i + *off]);
                }
        }
        strcpy(encrypted[count], res);
        printk("%d, Enc msg[%d]: %s\n", *off, count, encrypted[count]);
        *off += nb_bytes_to_copy;
        count = count + 1;
        return nb_bytes_to_copy;
}

static int dev_close(struct inode *inod, struct file *fil)
{
        printk(KERN_INFO "encdev: Device closed.\n");
        return 0;
}

module_init(encdev_init);
module_exit(encdev_exit);
