#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>


//Prototypes
void lsh_loop();
bool validateCommand(char c);
char *lsh_read_line();
char **lsh_split_line(char *line);
int lsh_execute(char **args);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_touch();
int lsh_joke();
int lsh_pipe(char *line);

//Globals
#define LSH_TOK_BUFSIZE 100
#define LSH_TOK_DELIM " \t\r\n\a"
#define LSH_RL_BUFSIZE 100
bool isPipe;


/**
 * This is the main function that drives the program. It boots up any
 * files or libraries that are needed to run the program and calls
 * the primary function for the program.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS
 */
int main(int argc, char **argv)
{


    // Load config files, if any.

    // Run command loop.

    lsh_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}

/**
 * This is the primary function for the program. It reads, analyzes, and
 * executes user inputs. For each of the three primary objectives of this
 * function a corresponding function is called to complete that objective.
 */
void lsh_loop()
{
    char *line;
    char **args;
    int status;


    do
    {
        printf("Jack and Andrews Shell \n"
               " ░░░░░░███████ ]▄▄▄▄▄▄▄▄▃\n"
               "▂▄▅█████████▅▄▃▂\n"
               "I███████████████████].\n"
               "◥⊙▲⊙▲⊙▲⊙▲⊙▲⊙▲⊙◤... \n"
               "\n");
        printf("\n");
        line = lsh_read_line();
        if(isPipe)
        {
            lsh_pipe(line);
        }
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);

    } while (status);
}


/**
 * This function is responsible for reading in user input. If the user inputs
 * 100 characters the method outputs an error message.
 *
 * Reference: https://dev.to/quantumsheep/basics-of-multithreading-in-c-4pam
 */
char *lsh_read_line()
{

    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    int c;
    char *buffer = malloc(sizeof(char) * bufsize);

    pthread_t mainThread;
    pthread_t secondThread;


    if (!buffer)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();


        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else if(c == '|')
        {
            isPipe = true;
            buffer[position] = c;
        }

        else if(!validateCommand(c) )
        {
            printf("This is not a valid command");
            exit(EXIT_FAILURE);
        }
        else if(c == '&')
        {

            buffer[position] = c;
            pthread_create(&secondThread, NULL, lsh_loop, NULL);
            pthread_join(mainThread, NULL);
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize)
        {
            printf("Your input was too long");

        }

    }
}

/**
 * This method is called to split the user input up to identify the
 * arguments that are in the line of text.
 * @param line
 * @return tokens
 */

char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            printf("Input too long idiot.");
            exit(EXIT_FAILURE);
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


char *builtin_str[] = {
        "cd",
        "help",
        "exit",
        "touch",
        "tell_joke"
};

int (*builtin_func[]) (char **) = {
        &lsh_cd,
        &lsh_help,
        &lsh_exit,
        &lsh_touch,
        &lsh_joke
};

//I also have no clue what this does
int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/**
 * This method executes the user input. It returns ....
 * @param args
 * @return
 */
int lsh_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}

/**
 * Duplicates the current process with the same parameters.
 * @param args
 * @return OBTAIN_INPUT
 */
int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;
    const int OBTAIN_INPUT = 1;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0)
    {
        // Error forking
        perror("lsh");
    } else
    {
        // Parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return OBTAIN_INPUT;
}

/**
 * This method is for internal process. It will change the directory
 * to the specified directory.
 * @param args
 * @return
 */
int lsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else
    {
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}

/**
 * This method is for internal process. It will display a list of
 * commands and their usage to the user.
 * @param args
 * @return
 */
int lsh_help(char **args)
{
    int i;
    printf("Jack and Andrews shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("These are the built in functions:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");

    return 1;
}

/**
 * This method will check each character that the user enters for errors.
 * If the user enters an invalid command an error message is displayed.
 * @param c
 * @return validCommand
 */
bool validateCommand(char c){

    bool validCommand = false;

    if( isalpha(c) )
    {
        validCommand = true;

    }
    else if (isalnum(c) )
    {
        validCommand = true;

    }
    else if( c == '.' )
    {
        validCommand = true;
    }
    else if( c == '/')
    {
        validCommand = true;
    }
    else if( c == '_' )
    {
        validCommand = true;
    }
    else if( c == '-' )
    {
        validCommand = true;
    }
    else if ( c == ';')
    {
        validCommand = true;
    }
    else if( c == '|')
    {
        validCommand = true;
    }
    else if( c == ' ' )
    {
        validCommand = true;
    }
    else if( c == '&' )
    {
        validCommand = true;
    }
    else
    {
        validCommand = false;

    }

    return validCommand;
}

/**
 * This function will create a new file. It asks the user to enter
 * in the name of the file and then creates a new file. The user must
 * specify the extentsion of the file.
 *
 */
int lsh_touch()
{

    char fileName[LSH_RL_BUFSIZE];
    FILE *newFile;

    printf("Please enter in the name of the file, \n"
           "YOU MUST SPECIFIY THE FILE EXTENSIION! \n");

    scanf("%s", fileName);
    newFile = fopen(fileName, "w");


}

/**
* This is an internal proccess that will tell a joke. It generates
* a random number that determines what joke will be told.
*
*/
int lsh_joke()
{

    printf("\n"
           "I wasn't sure if this should be an external or internal proccess, but even if it was an"
           "external proccess it would still be the same kind of joke if you know what I mean "
           "\n" "\e[1m tinyurl.com/GoodMeme123\e[0m"
           "\n" "\n");

}

/**
 * This is an internal process that will execute the pipe command.
 *
 * Reference:
 * */
int lsh_pipe(char *line)
{
    int fd[2];
    int bufsize = LSH_TOK_BUFSIZE;
    char** args;

    args = malloc(bufsize * sizeof(char*));
    char* token = malloc(bufsize *sizeof(char*));

    token = strtok(line, "|");

    args = lsh_split_line(token);

    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Fork Failed");
        exit(-1);

    }

    int pid = fork();

    if(pid < 0){
        return 2;
    }

    if (pid == 0)
    {
        printf("Process 1 started: ");
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(args[0], args);

    }

    int pidp = fork();

    if(pidp < 0){
        printf("Process 2 started: ");
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(args[1], args);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid, NULL, 0);
    waitpid(pidp, NULL, 0);


}


/**
 * This is for internal process. It will exit the program.
 * @param args
 * @return
 */
int lsh_exit(char **args)
{
    return 0;
}