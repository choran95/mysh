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

// Initalize variables.
int backgroundCount = 0;
pid_t pid;
char *getcwd(char *buf, size_t size);
int chdir(const char *path);
char *path;
char *builtIn[] = {"pwd", "cd", "show-dirs", "show-files", 
			"mkdir", "tool", "clear", "exit", "wait"};
char cmd[512];
char cwd[512];

// Throws the one true error message.
void throwError() {
	char error_message[30] = "An error has occured\nmysh> ";
				write (STDERR_FILENO, error_message, strlen(error_message));

}

// Gets the last index of an array.
int top(char *array[512]) 
{
    int i;
    for(i = 0; array[i] != '\0'; i++);
    	return --i;
}

// Checks wheather or no the command is built-in.
bool isBuiltIn (char *array1[512], char *array2[9]) {
	for (int i = 0; i < 9; i++){
		if (strcmp (array1[0], array2[i]) == 0){
			return true;
		}	
	}
	return false;

}

// Checks if the input needs a redirection.
bool isRedirect (char *array[512]) {
	if (top(array) == 0) {
		return false;
	}
	else if (strcmp (array[top(array)-1], ">") == 0) {
		return true;
	}
	else if ((top(array)-1) == 0) {
		return false;
	}
	else if (strcmp (array[top(array)-2], ">") == 0){
		return true;
	}
	else {
		return false;
	}

}

// Checks if the input is a background process.
bool isBackground (char *array[512]) {
	if (strcmp (array[top(array)], "&") == 0) {
		return true;
	}
	else {
		return false;
	}

}

// Checks if the input is a python file.
bool isPython(const char *file) {
	if ((file[strlen(file)-2] == 'p') && (file[strlen(file)-1] == 'y')) {
		return true;
	}
	return false;
}

// Checks whether or not the input has valid syntax for redirection.
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

// Checks whether or not the input has valid syntax for a background process.
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

// Concatenates two strings.
char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);
    return result;
}

bool inputCMD (char *cmdName) {	
	char *name;
	char *array[512] = {NULL};
        int i = 0;
        DIR *d;
        d = opendir(".");
        struct dirent *dir;
	if (cmdName == NULL) {
		name = malloc (512);
		fgets (name, 512, stdin);
		char *p = strtok(name," \n");
        	while(p != NULL) {
            		array[i++] = p;
            		p = strtok (NULL, " \n");
        	}		
	}
	else {
		char *p = strtok(cmdName," \n");
        	while(p != NULL) {
            		array[i++] = p;
            		p = strtok (NULL, " \n");
        	}
	}
	if (validRedirect(array) && validBackground(array) && (array[0] != NULL)) {
		strcpy(cmd, array[0]);
		if (isBuiltIn(array, builtIn)) {
			if (strcmp (array[0], "pwd") == 0) {
            			path = getcwd(cwd, sizeof(cwd));
            			printf("%s\n", path);
        		}
			else if (strcmp (array[0], "cd") == 0){
	    			if (array[1] == NULL){
					if (chdir(getenv("HOME")) != 0){
                				printf("Error cd\n");
              
            				}
            				else {
                				chdir(getenv("HOME"));
					}
            			}
				else {
					getcwd(cwd, sizeof(cwd));
					chdir(concat((concat(cwd,"/")), array[1]));
	    			}
			}
			else if (strcmp (array[0], "show-dirs") == 0){
            			while ((dir = readdir(d)) != NULL) {
					if (dir->d_type == DT_DIR){
                				printf("%s\n", dir->d_name);
             				}
            			}
        		}
			else if (strcmp (array[0], "show-files") == 0){
            			while ((dir = readdir(d)) != NULL) {

					if (dir->d_type == DT_REG){
                				printf("%s\n", dir->d_name);
             				}
            			}
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
        		}
			else if ((strcmp (array[0], "tool") == 0) && (strcmp (array[1], "") != 0)){
            			FILE *fptr;
            			fptr = fopen(array[1], "rb+");
            			if(fptr == NULL) {
                			fptr = fopen(array[1], "wb");
            			}
        		}
			else if(strcmp (array[0], "clear") == 0) {
            			for (int i = 0; i < 100; i++) {
                			printf("\n");
            			}
        		}
			else if(strcmp (array[0], "exit") == 0) {
            			return false;
        		}
			else if (strcmp (array[0], "wait") == 0) {
				/* Wait for children to exit. */
				int status;
				pid_t pid;
				printf("%d\n", backgroundCount);
				while (backgroundCount > 0) {
  					pid = wait(&status);
  					//printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);			
					--backgroundCount;
				}
				backgroundCount = 0;
				
			}

			if (cmdName == NULL) {
				write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
				free(name);
			}
            

        	}
		else if (isPython(array[0])) {
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char *modArray[512];
				modArray[0] = "python";
				for (int i = 0; array[i] != '\0'; i++) {
					modArray[i+1] = array[i];

				}
				if (execvp("python", modArray) == -1){
					throwError();
				}
			}
			if (pid > 0) {
				wait(NULL);
				
				if (cmdName == NULL) {
					free(name);
					write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
				}
			}
		}
		else if (isRedirect(array) && isBackground(array)){
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char* fileName = array[top(array)-1];
				char *modArray[512];
				for (int i = 0; (strcmp (array[i], ">") != 0); i++) {
					modArray[i] = array[i];

				}
				int filefd = open(fileName, O_WRONLY|O_CREAT, 0666);
  				close(1);//Close stdout
  				dup(filefd);
				if (execvp(cmd, modArray) == -1){
					throwError();
				}
  				close(filefd);
				backgroundCount++;

			}
			if (pid > 0) {
				//wait(NULL);
				if (cmdName == NULL) {
					free(name);
					write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
				}
			}
		}
		else if (isRedirect(array)){
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char* fileName = array[top(array)];
				char *modArray[512];
				for (int i = 0; (strcmp (array[i], ">") != 0); i++) {
					modArray[i] = array[i];

				}
				int filefd = open(fileName, O_WRONLY|O_CREAT, 0666);
  				close(1);//Close stdout
  				dup(filefd);
				if (execvp(cmd, modArray) == -1){
					throwError();
				}
  				close(filefd);

				

			}
			if (pid > 0) {
				wait(NULL);
				if (cmdName == NULL) {
					free(name);
					write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
				}
			}
		}
		else if (isBackground(array)) {
			pid = fork();
			backgroundCount++;
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char *modArray[512];
				for (int i = 0; (strcmp (array[i], "&") != 0); i++) {
					modArray[i] = array[i];

				}
				if (execvp(cmd, modArray) == -1){
					throwError();
				}
				
			
			}
			if (pid > 0) {

				if (cmdName == NULL) {
					free(name);
					write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
				}
			}

		}
		else {
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				if (execvp(cmd, array) == -1){
					throwError();
				}
			}
			if (pid > 0) {
				wait(NULL);
				
				if (cmdName == NULL) {
					free(name);
					write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
				}
			}
		}
	}
	else {
		throwError();
	}
	return true;

}

int main(int argc, char *argv[]) {
    if (argc > 2) {
	char error_message[30] = "An error has occured\n";
	write (STDERR_FILENO, error_message, strlen(error_message));
	return 0;
    }

    if (argc == 2) {
	FILE * fp;
    	char * line = NULL;
    	size_t len = 0;
    	ssize_t read;
	getcwd(cwd, sizeof(cwd));

    	fp = fopen(concat((concat(cwd,"/")), argv[1]), "r");
    	if (fp == NULL) {
        	exit(EXIT_FAILURE);
	}

    	while ((read = getline(&line, &len, fp)) != -1) {
		write(STDOUT_FILENO, line, strlen(line));
        	inputCMD(line);
   	}

    	fclose(fp);
   	if (line) {
        	free(line);
	}
	//write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
	while (inputCMD(NULL));


    }
	else {
		write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
		while (inputCMD(NULL));
	}
	
    return 0;
}
