#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    char str[4096];
    int rc;
    int fd;
    int i;
    fd = open("input", O_RDONLY);   
   
    while(1) {
	rc = read(0, str, 4096);	
	str[rc] = '\0';

	//for(i = 0; i < 100; i++) 
	//write(1, "int = 79\nRadclient-Query-Id = \"466\"\n\n", strlen(str));
	write(1, str, strlen(str));
 	
	    //rc = read(fd, str, 4096);
	    //str[rc] = '\0';

	
	//	write(2, str, strlen(str));
	
    }
    
    close(fd);
    
    return 0;
}
