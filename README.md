/************* BLOCK DEVICE DRIVER *****************/

RUNNING THE CODE:

0) Open terminal

1) change working directory(pwd), to directory with the main.c and Makefile

    $ cd  <files_directory>

2) Type command:  'make' (to generate necessary modules)

      $ make

3) To insert module type 

    $  sudo insmod main.ko 

4) verify inserted Kernel module with any of commands: 

    $ dmesg 

    $ lsmod

5) Check the partitions using the command
    
    $ sudo fdisk -l /dev/dof   
6) Change permissions of the registered device.

    $ sudo chmod 777 /dev/dof

6) To get details about the block use diskdump utility 
    
    $ dd if=/dev/dof of=dof

7) Reading or writing to the device can be done as follows
   
  i) Change  the permissions using the command  
    $ sudo chmod 777 /dev/dof1
   
   ii) write data into the block using the command 
    $ cat > /dev/dof1
    
   iii) Type anything in the terminal " Hello World "

   iv) To display the  written data in the block using the command

    $ xxd /dev/dof1 | less
   
8) To remove the block driver use the command 

    $ sudo rmmod main.ko

/********************* END ************************/
             

