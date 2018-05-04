# Encryption-Decryption
An encryption-decryption Linux kernel module.

## Operating Systems Course
(Group No.-32)
Ritu Kumari, 2016078
Sneha Sinha, 2016098

#### Commands: 

##### 1. For encdev character device -
	sudo mknod /dev/encdev c 89 1
	sudo chmod a+r+w /dev/encdev
	
##### 2. For decdev character device -
	sudo mknod /dev/decdev c 89 1
	sudo chmod a+r+w /dev/decdev
  
##### 3. make (command)
	1) to compile the driver and generate .ko files.
	2) compile the test files.
	
##### 4. To load the modules
	sudo insmod encdev.ko
	sudo insmod decdev.ko
	
##### 5. To unload the modules
	sudo rmmod encdev
	sudo rmmod decdev 

##### 6. To check if encdev or decdev is currently loaded or not
	lsmod | grep encdev
	lsmod | grep decdev
	
##### 7. To run the test files -
	1. For encdev
        ./test [input filename] [output filename]
	2. For decdev
        ./test [output filename]
	
- If the test function show sigkill, reload the module.
Proceed as : unloading, making and loading of module.

- Input and output files are ASCII files.

#### Encdev:
    1. When writing to device, check if it is the first write to device (use a flag variable to check) and if it is then we're getting the key from the /dev/urandom.
    2. Saved that in a variable for the key.
    3. Then, created another array to store current encrypted data.
    4. For the first write, XOR the input data with the key and save that in encrypted data array.
    5. Whenever there is a subsequent write to the device, XOR the input data with the last encrypted item and save the current one in the encrypted data array.
    6. Now, when there's a read from device, first send the key to the user and then send encrypted data array.

#### TestEncDev:
    1. Input file contains the message to be encrypted, passed as argument.
    2. The generated key and encrypted data array is written to the output file, passed as argument.

#### Decdev:
    1. When writing to device, check if it is the first write to device (use a flag variable to check) and if it is then we're getting the key from the file passed as input to the testdecdev.
    2. Saved that in a variable for the key.
    3. Then, created another array to store current decrypted data.
    4. For the first write, XOR the input data with the key and save that in decrypted data array.
    5. Whenever there is a subsequent write to the device, XOR the input data with the last decrypted item and save the current one in the decrypted data array.
    6. Now, when there's a read from device, first send the decrypted data array to the user.

#### TestDecDev:
    1. The argument file contains the encrypted message previously written by the TestEncDev.
    2. Displays the decrypted message, same as the contents of the Input file.
