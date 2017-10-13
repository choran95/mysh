#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int top(char *array[512]) 
{
    int i;
    for(i = 0; array[i] != '\0'; i++);
    	return --i;
}


bool isBuiltIn (char *array1[512], char *array2[8]) {
	for (int i = 0; i < 8; i++){
		if (strcmp (array1[0], array2[i]) == 0){
			return true;
		}	
	}
	return false;

}

bool isRedirect (char *array[512]) {
	for (int i = 0; i < 512; i++){
		if (strcmp (array[i], ">") == 0){
			return true;
		}	
	}
	return false;

}

bool validRedirect (char *array[512]) {
		int count = 0;
		int count2 = 0;
		int index = 0;
		for (int i = 0; array[i] != '\0'; i++){
			if (strcmp (array[i], ">") == 0){
				count++;
				index = i;
			}	
		}
		if (count == 0){
			return true;
		}
		for (int j = index + 1; array[j] != '\0'; j++){
			if ((strcmp (array[j], "&")) != 0) {
				count2++;
				
			}
		}
		if (count == 1 && count2 == 1){
			return true;
		}
		else {
			return false;
		}
}

bool validBackground (char *array[512]) {
		int count = 0;
		int count2 = 0;
		int index = 0;
		for (int i = 0; array[i] != '\0'; i++){
			if ((strcmp (array[i], "&")) == 0){
				count++;
				index = i;
			}	
		}
		if (count == 0) {
			return true;
		}
		else if ((count == 1) && (top(array) == index)){
			return true;
		}
		return false;


}


char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);
    return result;
}

int main( ) {

    pid_t pid;
    char *getcwd(char *buf, size_t size);
    int chdir(const char *path);
    char *path;
    char *builtIn[] = {"pwd", "cd", "show-dirs", "show-files", 
			"mkdir", "tool", "clear", "exit"};
    char cmd[512];
    char cwd[512];
    printf("mysh > ");

    while (strcmp (cmd, "exit") != 0){
        char *name = malloc (512);
        char *array[512] = {NULL};
        int i = 0;
        DIR *d;
        d = opendir(".");
        struct dirent *dir;
        fgets (name, 512, stdin);
        char *p = strtok(name," \n");

        while(p != NULL)
        {
            array[i++] = p;
            p = strtok (NULL, " \n");
        }
        strcpy(cmd, array[0]);
	if (validRedirect(array) && validBackground(array)) {
		if (isBuiltIn(array, builtIn)) {
			if (strcmp (array[0], "pwd") == 0) {
            			path = getcwd(cwd, sizeof(cwd));
            			printf("%s\n", path);
            			printf("mysh > ");
        		}
			else if (strcmp (array[0], "cd") == 0){
	    			if (array[1] == NULL){
					if (chdir(getenv("HOME")) != 0){
                				printf("Error cd\n");
                				printf("mysh > ");
            				}
            				else {
                				printf("mysh > ");
                				chdir(getenv("HOME"));
					}
            			}
				else {
					getcwd(cwd, sizeof(cwd));
					chdir(concat((concat(cwd,"/")), array[1]));
					printf("mysh > ");
	    			}
			}
			else if (strcmp (array[0], "show-dirs") == 0){
            			while ((dir = readdir(d)) != NULL) {
					if (dir->d_type == DT_DIR){
                				printf("%s\n", dir->d_name);
             				}
            			}
	   			printf("mysh > ");
        		}
			else if (strcmp (array[0], "show-files") == 0){
            			while ((dir = readdir(d)) != NULL) {

					if (dir->d_type == DT_REG){
                				printf("%s\n", dir->d_name);
             				}
            			}
	    			printf("mysh > ");
        		}	
	    		else if ((strcmp (array[0], "mkdir") == 0) && (strcmp (array[1], "") != 0)){
            			getcwd(cwd, sizeof(cwd));
            			path = concat((concat(cwd,"/")), array[1]);
				DIR* dir = opendir(path);
            			if (dir) {
                			printf("%s already exists.\n", array[1]);
            			}
            			else {
                			mkdir(path, 0777);
            			}		
            			printf("mysh > ");
        		}
			else if ((strcmp (array[0], "tool") == 0) && (strcmp (array[1], "") != 0)){
            			FILE *fptr;
            			fptr = fopen(array[1], "rb+");
            			if(fptr == NULL) {
                			fptr = fopen(array[1], "wb");
            			}
            			printf("mysh > ");
        		}
			else if(strcmp (array[0], "clear") == 0) {
            			for (int i = 0; i < 100; i++) {
                			printf("\n");
            			}
            			printf("mysh > ");
        		}
			free(name);

            

        	}
		else {

			pid = fork();
			if (pid == 0 && (isRedirect(array))) {
				char* fileName = array[top(array)];
				char *modArray[512];
				for (int i = 0; (strcmp (array[i], ">") != 0); i++) {
					modArray[i] = array[i];

				}
				int filefd = open(fileName, O_WRONLY|O_CREAT, 0666);
  				close(1);//Close stdout
  				dup(filefd);
				execvp(cmd, modArray);
  				close(filefd);
				//wait(NULL);

			}

			//wait(NULL);
			printf("mysh > ");
			free(name);
		}

	}
	else {
		char error_message[30] = "An error has occured\n";
		write (STDERR_FILENO, error_message, strlen(error_message));
		printf("mysh > ");
	}
    }
	
    return 0;
}
