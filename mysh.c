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
struct Node *start = NULL;
pid_t pid;
char *getcwd(char *buf, size_t size);
int chdir(const char *path);
char *path;
char *builtIn[] = {"pwd", "cd", "show-dirs", "show-files", 
			"mkdir", "tool", "clear", "exit", "wait", "mysh"};
char cmd[513];
char cwd[513];

struct command
{
  const char **argv;
};


int spawn_proc (int in, int out, struct command *cmd) {
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}


int fork_pipes (int n, struct command *cmd) {
  int i;
  pid_t pid;
  int in, fd [2];

  in = 0;

  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);

      spawn_proc (in, fd [1], cmd + i);

      close (fd [1]);

      in = fd [0];
    }

  
  if (in != 0)
    dup2 (in, 0);

  
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
}

// Throws the one true error message.
void throwError() {
	char error_message[30] = "An error has occured\nmysh> ";
				write (STDERR_FILENO, error_message, strlen(error_message));

}

void throwPipeError() {
	char error_message[30] = "An error has occured\n";
				write (STDERR_FILENO, error_message, strlen(error_message));

}

// Gets the last index of an array.
int top(char *array[513]) 
{
    int i;
    for(i = 0; array[i] != '\0'; i++);
    	return --i;
}

/* A linked list node */
struct Node
{
    // Any data type can be stored in this node
    void  *data;
 
    struct Node *next;
};
 
/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void push(struct Node** head_ref, void *new_data, size_t data_size)
{
    // Allocate memory for node
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
 
    new_node->data  = malloc(data_size);
    new_node->next = (*head_ref);
 
    // Copy contents of new_data to newly allocated memory.
    // Assumption: char takes 1 byte.
    int i;
    for (i=0; i<data_size; i++)
        *(char *)(new_node->data + i) = *(char *)(new_data + i);
 
    // Change head pointer as new node is added at the beginning
    (*head_ref)    = new_node;
}

/* Function to wait on nodes in a given linked list. fpitr is used
   to access the function to be used for waiting on current node data.
   Note that different data types need different specifier in printf() */
void waitList(struct Node *node, void (*fptr)(void *)) {
    while (node != NULL) {
        (*fptr)(node->data);
        node = node->next;
    }
}

// Function to wait on pid.
void waitAll(void *n) {
	waitpid(*(int *)n, NULL, WNOHANG);
}
 

// Checks wheather or no the command is built-in.
bool isBuiltIn (char *array1[513], char *array2[10]) {
	for (int i = 0; i < 10; i++){
		if (strcmp (array1[0], array2[i]) == 0){
			return true;
		}	
	}
	return false;

}

// Checks if the input needs a redirection.
bool isRedirect (char *array[513]) {
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

bool isPipe (char *array[513]) {
	for (int i = 0; array[i] != '\0'; i++){
			if (strcmp (array[i], "|") == 0){
				return true;
			}	
		}
	return false;
}

// Checks if the input is a background process.
bool isBackground (char *array[513]) {
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
bool validRedirect (char *array[513]) {
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
bool validBackground (char *array[513]) {
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
	
	unsigned int_size = sizeof(int);
	bool isCorrectSize = true;
	char *name;
	char *array[513] = {NULL};
        int i = 0;
        DIR *d;
        d = opendir(".");
        struct dirent *dir;
	if (cmdName == NULL) {
		name = malloc (513);
		 if(fgets(name,513,stdin)){
            		char *p;
            		if(p=strchr(name, '\n')){//check exist newline
                		*p = 0;
				char *p = strtok(name," \n");
        			while(p != NULL) {
            				array[i++] = p;
            				p = strtok (NULL, " \n");
        			}
			}
			else {
				scanf("%*[^\n]");scanf("%*c");
				isCorrectSize = false;
			}
		}		
	}
	else {
		char *p = strtok(cmdName," \n");
        	while(p != NULL) {
            		array[i++] = p;
            		p = strtok (NULL, " \n");
        	}
	}
	if (validRedirect(array) && validBackground(array) && (array[0] != NULL) && (isCorrectSize)) {
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
				waitList(start, waitAll);
				start = NULL;

				
			}
			else if (strcmp (array[0], "mysh") == 0) {
				char *p = getenv("USER");
    				if(p == NULL) {
					printf("N/A");
				}
    				printf("%s\n",p);

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
				char *modArray[513];
				modArray[0] = "python";
				for (int i = 0; array[i] != '\0' && (strcmp (array[i], "&") != 0); i++) {
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
		else if (isRedirect(array) && isPipe(array)) {
			bool isBack = isBackground(array);
			bool pipeError = false;
			struct command pipeCmd [513];
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char* fileName;
				char *modArray[513][513];
				if (isBack) {
					fileName = array[top(array)-1];
				}
				else { 
					fileName = array[top(array)];
				}
				int arrayNumber = 0;
				int startIndex = 0;
				for (int i = 0; (strcmp (array[i], ">") != 0); i++){
					if (strcmp (array[i], "|") != 0) {
						modArray[arrayNumber][startIndex] = array[i];
						startIndex++;
					}
					else {
		
						modArray[arrayNumber][startIndex] = NULL;
						startIndex = 0;
						pipeCmd[arrayNumber].argv = modArray[arrayNumber];
						arrayNumber++;
						
					}
				}
				modArray[arrayNumber][startIndex] = NULL;
				pipeCmd[arrayNumber].argv = modArray[arrayNumber];
				int filefd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
  				close(1);//Close stdout
  				dup(filefd);
				if (isBack) {
					push(&start, &pid, int_size);
				}
				if (fork_pipes (arrayNumber+1, pipeCmd) < 0) {
					pipeError = true;
					throwPipeError();
					return false;
				}
				close(filefd);
			}
			if (pid > 0) {
				if (!isBack) {
					wait(NULL);
				}
				if (cmdName == NULL) {
					free(name);
					if (!pipeError){
						write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
					}
				}
			}
		}
		else if (isPipe(array)) {

			bool pipeError = false;
			bool isBack = isBackground(array);
  			struct command pipeCmd [513];
			
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char *modArray[513][513];
				int arrayNumber = 0;
				int startIndex = 0;
				for (int i = 0; array[i] != NULL && (strcmp (array[i], "&") != 0); i++){
					if (strcmp (array[i], "|") != 0) {
						modArray[arrayNumber][startIndex] = array[i];
						startIndex++;
					}
					else {
		
						modArray[arrayNumber][startIndex] = NULL;
						startIndex = 0;
						pipeCmd[arrayNumber].argv = modArray[arrayNumber];
						arrayNumber++;
						
					}
				}
				modArray[arrayNumber][startIndex] = NULL;
				pipeCmd[arrayNumber].argv = modArray[arrayNumber];
				if (isBack) {
					push(&start, &pid, int_size);
				}
				if (fork_pipes (arrayNumber+1, pipeCmd) < 0) {
					pipeError = true;
					throwPipeError();
					return false;
				}
			}
			if (pid > 0) {
				if (!isBack) {
					wait(NULL);
				}
				if (cmdName == NULL) {
					free(name);
					if (!pipeError){
						write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
					}
				}
			}
		}
		else if (isRedirect(array)){
			bool isBack = isBackground(array);
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char* fileName;
				if (isBack) {
					fileName = array[top(array)-1];
				}
				else { 
					fileName = array[top(array)];
				}
				char *modArray[513];
				for (int i = 0; (strcmp (array[i], ">") != 0); i++) {
					modArray[i] = array[i];

				}
				int filefd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
  				close(1);//Close stdout
  				dup(filefd);
				if (isBack) {
					push(&start, &pid, int_size);
				}
				if (execvp(cmd, modArray) == -1){
					throwError();
				}
  				close(filefd);

				

			}
			if (pid > 0) {
				if (!isBack) {
					wait(NULL);
				}
				if (cmdName == NULL) {
					free(name);
					write (STDERR_FILENO, "mysh> ", strlen("mysh> "));
				}
			}
		}
		else if (isBackground(array)) {
			pid = fork();
			if (pid < 0) {
				throwError();
			}
			if (pid == 0) {
				char *modArray[513];
				for (int i = 0; (strcmp (array[i], "&") != 0); i++) {
					modArray[i] = array[i];

				}
				push(&start, &pid, int_size);
				if (execvp(cmd, modArray) == -1){
					throwError();
				}
				
			
			}
			if (pid > 0) {
				//printf("%d", pid);

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
