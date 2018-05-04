#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include<linux/slab.h>
#include<linux/slab.h>
#include<linux/random.h>

#define DEVICE_NAME "decdev"
#define CLASS_NAME "dec"
#define BUFFER_SIZE 256

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("decdev driver demo");

static char message[BUFFER_SIZE] = {0};
static char decrypted[BUFFER_SIZE][BUFFER_SIZE];
static int count = 0, readCount = 0;
char randomKey[16];
static struct class* decdevClass = NULL;
static struct device* decdevDevice = NULL;

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

static int __init decdev_init(void)
{
        int t = register_chrdev(90, DEVICE_NAME, &fops);
        if (t < 0)
        {
                printk(KERN_INFO "decdev: Device registration failed.\n");
                return t;
        }
        printk(KERN_INFO "decdev: Device registered successfully.\n");

        decdevClass = class_create(THIS_MODULE, CLASS_NAME);
        if (IS_ERR(decdevClass))
        {
                unregister_chrdev(90, DEVICE_NAME);
                printk(KERN_INFO "decdev: Failed to register device class.\n");
                return PTR_ERR(decdevClass);
        }
        printk(KERN_INFO "decdev: Device class registered successfully.\n");

        decdevDevice = device_create(decdevClass, NULL, MKDEV(90, 0), NULL, DEVICE_NAME);
        if (IS_ERR(decdevDevice))
        {
                class_destroy(decdevClass);
                unregister_chrdev(90, DEVICE_NAME);
                printk(KERN_INFO "decdev: Failed to create the device.\n");
                return PTR_ERR(decdevDevice);
        }
        printk(KERN_INFO "decdev: Device class created successfully.\n");
        return 0;
}

static void __exit decdev_exit(void)
{
        device_destroy(decdevClass, MKDEV(90, 0));
        class_unregister(decdevClass);
        class_destroy(decdevClass);
        unregister_chrdev(90, DEVICE_NAME);
        printk(KERN_INFO "decdev: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inod, struct file *fil)
{
        printk(KERN_INFO "decdev: Device opened.\n");
        return 0;
}

static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
        int times = 0, readPos = 0;
        if (readCount == 0)
        {
                printk("Key: %s\n", randomKey);
                while (len && randomKey[readPos]!='\0')
                {
                        put_user(randomKey[readPos], buff++);
                        len--;
                        times++;
                        readPos++;
                }
                readCount = readCount + 1;
                return times;
        }
        int track = readCount - 1;
        printk("%d, Dec msg[%d]: %s\n", *off, track, decrypted[track]);
        while (len && decrypted[track][readPos]!='\0')
        {
                put_user(decrypted[track][readPos], buff++);
                len--;
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
        if (count > 0)
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
                strcpy(randomKey, message);
                printk(KERN_INFO "Key: %s\n", randomKey);
        }
        else if (count == 1)
        {
                for (i = 0; i < length; i++)
                {
                        res[i] = (char)(randomKey[i] ^ message[i + *off]);
                }
                strcpy(decrypted[track], res);
                printk("%d, Dec msg[%d]: %s\n", *off, track, decrypted[track]);
        }
        else
        {
                for (i = 0; i < length; i++)
                {
                        res[i] = (char)(decrypted[track - 1][i] ^ message[i + *off]);
                }
                strcpy(decrypted[track], res);
                printk("%d, Dec msg[%d]: %s\n", *off, track, decrypted[track]);
        }
        *off += nb_bytes_to_copy;
        count = count + 1;
        return nb_bytes_to_copy;
}

static int dev_close(struct inode *inod, struct file *fil)
{
        printk(KERN_INFO "decdev: Device closed.\n");
        return 0;
}

module_init(decdev_init);
module_exit(decdev_exit);
