/*
 *  Filename : stream.c
 * 	Purpose	 : routines for stream read and write
 *	Author 	 : Aung Aye Than and Tan Yeow Keat
 *  Date  	 : 25/11/2017
 *  Version  : 1.0
 *	 	. 
 */

#include  <sys/types.h>
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include  "stream.h"
#include  <stdlib.h>
#include  <unistd.h>

int readn(int fd, char *buf, int bufsize)//fd is socket, buf is pointer/buffer to read, and bufsize is size of buffer to read
{
    short data_size;    /* sizeof (short) must be 2 */ 
    int n, nr, len;

    /* check buffer size len */
    if (bufsize < MAX_BLOCK_SIZE)//bufsize being read is 
         return (-3);     /* buffer too small */

    /* get the size of data sent to me */
    if (read(fd, (char *) &data_size, 1) != 1) return (-1);    //read the amount of bytes to receive.
    if (read(fd, (char *) (&data_size)+1, 1) != 1) return (-1);//receive the actual message in accordance to the amount of bytes to receive.
    len = (int) ntohs(data_size);  //convert to host byte order 

    /* read len number of bytes to buf */
    for (n=0; n < len; n += nr) {
        if ((nr = read(fd, buf+n, len-n)) <= 0) 
            return (nr);       /* error in reading */
    }
    return (len);//return the amount of characters/bytes read if read is successful.
}

int writen(int fd, char *buf, int nbytes)//fd is the socket, buf is the pointer/buffer to read, and nbytes is the length to write into socket
{
    short data_size = nbytes;     /* short must be two bytes long */
    int n, nw;

    if (nbytes > MAX_BLOCK_SIZE) //nbytes being writen is larger than max block size because buffer created on client/server is MAX_BLOCK_SIZE
         return (-3);    /* too many bytes to send in one go */ 

    /* send the data size */
    data_size = htons(data_size); //convert to network byte order 
    if (write(fd, (char *) &data_size, 1) != 1) return (-1);    //send the size of the byte stream to the reader first.  
    if (write(fd, (char *) (&data_size)+1, 1) != 1) return (-1);//send the actual message in accordance to the byte size.

    /* send nbytes */
    for (n=0; n<nbytes; n += nw) {
         if ((nw = write(fd, buf+n, nbytes-n)) <= 0)  
             return (nw);    /* write error */
    } 
    return (n);//return the amount of characters/bytes writen if write is successful
}
