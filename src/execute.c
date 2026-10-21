/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>

#include <sys/wait.h> /*Implicit declaration of function 'open', 'O_RDONLY' undeclared*/

#include <signal.h> /*Implicit declaration of function 'kill', 'SIGINT' undeclared*/

#include <fcntl.h>

#include "quash.h"

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

#include "deque.h" //Generic double-ended queue macros from the starter code
IMPLEMENT_DEQUE_STRUCT(PidDeque, pid_t); //Generate a deque type named PidDeque that holds pid_t values
IMPLEMENT_DEQUE(PidDeque, pid_t);        //Generate its functions: new_PidDeque, push_back_PidDeque, pop_front_PidDeque, etc.

PidDeque pid_list;                       // PIDs of every process in the job currently being ran.
int pipes[2][2]; //two pipes that alternate: process i writes to pipes[i % 2] process i+1 reads from it ([0] = read end, [1] = write end)
int proc_index; //which command of the current job create_process is on (0,1,2, ...)

typedef struct Job {
  int job_id;    // The [1], [2], number shown to the user
  pid_t pid;    //PID of the job's first process (printed in messages)
  PidDeque pids; //PIDs of the job's processes that haven't finished yet
  char* cmd;    //What was typed. "sleep 15 &" (from get_command_string, must be freed)
} Job;

IMPLEMENT_DEQUE_STRUCT(JobDeque, Job); //Generate a deque type named JobDeque that holds Job values
IMPLEMENT_DEQUE(JobDeque, Job);        //Generate its functions: new_JobDeque, push_back_JobDeque

JobDeque jobs;           //All background jobs, oldest first
bool jobs_ready = false; //Whether the jobs list has been created yet

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  
  char* cwd = getcwd(NULL, 0); //Passing NULL, 0 tells getcwd to allocate memfory the string itself and the size it to fit
  *should_free = true; //Memory came from malloc so we have to free it or Valgrind reports a leak
  return cwd;

  // HINT: This should be pretty simple
 
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
    return getenv(env_var); //returns the value of an environment variable env_var
  
}
// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  if (!jobs_ready) { //First time quash runs anything
    jobs = new_JobDeque(1); //Create the empty job list
    jobs_ready = true; //Remember its created
  }
  size_t num_jobs = length_JobDeque(&jobs);     //How many of its processes are still being tracked
  for (size_t i = 0; i < num_jobs; i++) {        //Visit each process once
    Job job = pop_front_JobDeque(&jobs);       //Take the job off the front
    size_t num_pids = length_PidDeque(&job.pids);  //How many of its processes are still being touched
    for (size_t j = 0; j < num_pids; j++) {
      pid_t pid = pop_front_PidDeque(&job.pids);   //Take the PID off the front
      int status;                                  //Waitpid stores the exit info
      if (waitpid(pid, &status, WNOHANG) == 0) {  //WNOHANG = don't wait; retuns 0 if the process is still running
        push_back_PidDeque(&job.pids, pid);       //Still running: keep it (if finished, it's simply not put back)
    }
  }
  if (is_empty_PidDeque(&job.pids)) {                   //Every process in the job has finished
    print_job_bg_complete(job.job_id, job.pid, job.cmd);  //"Completed: [1] 2343 sleep 15 &"
    free(job.cmd);                                        //Free the strdup'd command string (prevents a Valgrind leaks)
    destroy_PidDeque(&job.pids);                          //Free the job's PID list
  }
  else {
    push_back_JobDeque(&jobs, job);                     //Still running: put the job back (in order)
  } 
}
  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  // TODO: Remove warning silencers
  

  // TODO: Implement run generic
  execvp(exec, args); //REPLACE this child process with program; searches PATH automatically

  perror("ERROR: Failed to execute program");
  exit(EXIT_FAILURE); //execvp only returns if it failed, so end this child instead of letting it keep running as a 2nd quash
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  //TODO
  for (int i = 0; str[i] != NULL; i++) { //Go through each word until we hit the NULL at the end
    printf("%s", str[i]);                 //Print the current word
    if (str[i+1] != NULL)                 //IF there's another word after this one
      printf(" ");                        //Print space in between
  }
  printf("\n");                           //End the line

  



  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  // TODO: Remove warning silencers
 

  // TODO: Implement export.
  // HINT: This should be quite simple.
  setenv(env_var, val, 1); // Set env_var to val. The 1 means overwrite it if it already exists
}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;

  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // TODO: Change directory


  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  char* path = realpath(dir, NULL);               //Turn what the user typed into a full path
  if (path == NULL) {                             //Realpath fails if the directory doesn't exist
    perror("ERROR: Failed to resolve path");      //Print why to stderr 
    return;                                       //Stop; nothing to free since path is NULL
  }
  if (chdir(path) != 0) {                         //Actually change quash's working directory; nonzero means it failed
    perror("ERROR: Failed to change directory");  //Print why to stderr
    free(path);                                   //Free the path before leaving (prevents valgrind)
    return;                                       //Stop
  }
  const char* old_pwd = getenv("PWD");            //The directory we were in before
  if (old_pwd != NULL) {                          //Only if PWD was set
    setenv("OLDPWD", old_pwd, 1);                 //save it as OLDPWD(must happen before PWD is overwritten)
  }               
  setenv("PWD", path, 1);                         //Update PWD to the new directory
  free(path);                                     //Done with malloc'd pat
  
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  
 

  // TODO: Kill all processes associated with a background job
  size_t num_jobs = length_JobDeque(&jobs); //How many background jobs there are
  for (size_t i = 0; i < num_jobs; i++) {   //Visit each job once
    Job job = pop_front_JobDeque(&jobs);   //Take the job off the front
    if (job.job_id == job_id) {           //Is this the job the user named?
      size_t num_pids = length_PidDeque(&job.pids);  //How many processes are in the job
      for (size_t j = 0; j < num_pids; j++) {     //Visit each process once
        pid_t pid = pop_front_PidDeque(&job.pids);   //Take the PID off the front
        kill(pid, signal);                           //Send it the signal (9 = SIGKILL)
        push_back_PidDeque(&job.pids, pid);         //Keep it in the list; check_jobs_bg_status removes it once it actually dies

      }

    }
    push_back_JobDeque(&jobs, job);         //Put the job back (in order)
  }
}


// Prints the current working directory to stdout
void run_pwd() {
  // TODO: Print the current working directory
  char* cwd = getcwd(NULL, 0); //Get the real current directory (malloc'd string)
  printf("%s\n", cwd);         //Print it 
  free(cwd);                   //free it to prevent valgrind leak

 

  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  // TODO: Print background jobs
  size_t num_jobs = length_JobDeque(&jobs);   //How many background jobs there are
  for (size_t i = 0; i < num_jobs; i++) {     //Visit each job once
    Job job = pop_front_JobDeque(&jobs);   //Take the job off the front
    print_job(job.job_id, job.pid, job.cmd);  //Print "[1] 2342 sleep 15 &"
    push_back_JobDeque(&jobs, job);          //Put it on the back, so after the loop the list is back in its original order

  }

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  int curr = proc_index % 2;      //Pipe this process writes to (process 0 -> pipe 0 -> process 1 -> pipe 1, process 2 -> pipe 0)
  int prev = (proc_index + 1) % 2; //Pipe this previous process wrote to (the other one)
  if (p_out) {                      //If this command output goes into pipe
    pipe(pipes[curr]);              //Create that pipe now before forking so the child inherits it
  }
  pid_t pid = fork();               //Clone quash; the child gets 0, the parent gets the child's PID
  if (pid == 0) {                   //In child process
    if (p_in) {                     //If input comes from the previous commands pipe
      dup2(pipes[prev][0], STDIN_FILENO);       //Make the pipe's read end our stdin
      close(pipes[prev][0]);                    //Close the original; stdin is now the copy
    }
    if (p_out) {                              //If output goes into the next commands pipe
      dup2(pipes[curr][1], STDOUT_FILENO);     //Make the pipe's write end our stdout
      close(pipes[curr][0]);                     //We never read from our own output pipe so it closes
      close(pipes[curr][1]);                    //Close the original; stdout is now the copy
    }
    if (r_in) {                               //"<file": read stdin from a file
      int fd = open(holder.redirect_in, O_RDONLY); //open the file for reading
      if (fd < 0) {
        perror("ERROR: Failed to open input file"); //print why to stderr
        exit(EXIT_FAILURE);                         //End this child
      }
      dup2(fd, STDIN_FILENO);                       //Make the file our stdin
      close(fd);                                    //Close the original
    }
    if (r_out) {                                    //">file" or ">> file" write stdout to a file
      int flags = O_WRONLY | O_CREAT | (r_app ? O_APPEND : O_TRUNC); // Write, create if missing then append (>>) or wipe it first (>)
      int fd = open(holder.redirect_out, flags, 0644); //open the file (0644 = owner read/write, others read)
      if (fd < 0) {
        perror("ERROR: Failed to open output file"); //print why to stderr
        exit(EXIT_FAILURE);                          //End this child
      }
      dup2(fd, STDOUT_FILENO);                      //Make the file our stdout
      close(fd);                                    //Close the orignal
    }
    child_run_command(holder.cmd);                  //Run the command (generic programs exec here)
    exit(EXIT_SUCCESS);                             //Child ends here
  }                       
  else {                                            //In the parent (quash)
    if (p_in) {                                      //the child now has its own copy of the previous pipes read end
      close(pipes[prev][0]);                        //so quash closes its copy
    }
    if (p_out) {                              //The child now has its own copy of this piepes write ned
      close(pipes[curr][1]);                   //So quash closes its copy of this pipes write end
    }
    push_back_PidDeque(&pid_list, pid);               //Remember the child's PID so run_script can wait on it 
    parent_run_command(holder.cmd);               //Run in quash itself(does nothing for other commands)
  }
  proc_index++;                              //Next call to create_process is the next command in the pipeline
}


// Run a list of commands
void run_script(CommandHolder* holders) {
  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;
  pid_list = new_PidDeque(1); //Start an empty PID list for this job
  proc_index = 0;        //The first command in this job is process 0
  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i]);

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    while (!is_empty_PidDeque(&pid_list)) {               //Until every process in the job has been waited on
      int status;                                         //waitpid stores the child's exit info here
      waitpid(pop_front_PidDeque(&pid_list), &status, 0); //Take the next PID off the list and wait for that process to finish
    }
    destroy_PidDeque(&pid_list);                          //Free the list's memory (prevents a Valgrind leak)
  }
  else {
    // A background job.
    // TODO: Push the new job to the job queue
    Job job;   //The new background job
    job.job_id = is_empty_JobDeque(&jobs) ? 1 : peek_back_JobDeque(&jobs).job_id + 1; //Like bash: 1 if no jobs, otherwise highest ID + 1 (The newest job is at the back)
    job.pid = peek_front_PidDeque(&pid_list);   //First process's PID, the one printed in messages
    job.pids = pid_list;                        //Hand the PID list to job (don't destroy itl the job owns it now)
    job.cmd = get_command_string();            //Copy of what was typed "sleep 15 &" (strdup'd; freed when the job completes)
    push_back_JobDeque(&jobs, job);            //Add it to the background job list
    print_job_bg_start(job.job_id, job.pid, job.cmd);   //Background job started: [1] 2343 sleep 15 &

    // TODO: Once jobs are implemented, uncomment and fill the following line
    // print_job_bg_start(job_id, pid, cmd);
  }
}
