#include "shell.hh"

int main() {
  std::string input;

  // ToDo: Vervang onderstaande regel: Laad prompt uit bestand
  std::string prompt = "ToDo: Laad de prompt uit een bestand! ";

  while (true) {
    std::cout << prompt;           // Print het prompt
    std::getline(std::cin, input); // Lees een regel
    if (input == "new_file")
      new_file(); // Kies de functie
    else if (input == "ls")
      list(); //   op basis van
    else if (input == "src")
      src(); //   de invoer
    else if (input == "find")
      find();
    else if (input == "seek")
      seek();
    else if (input == "exit")
      return 0;
    else if (input == "quit")
      return 0;
    else if (input == "error")
      return 1;

    if (std::cin.eof())
      return 0;
  }
} // EOF is een exit

// std::string Prompt(const std::string &filename) {
//   int fd = syscall(SYS_open, filename, O_RDONLY);
//   if (fd == -1) {
//     perror("Error Opening PromptFile");
//     return "exit";
//   }
//   char buffer[256];
//   ssize_t bytesRead = syscall(SYS_read, fd, buffer, sizeof(buffer) - 1);
//   if (bytesRead == -1) {
//     perror("Error reading prompt file");
//     syscall(SYS_close, fd);
//     return "Exit";
//   }
//   syscall(SYS_close, fd);
//   buffer[bytesRead] = "\0";
//   return std::string(buffer);
// }

bool endsWith(std::string file_text, std::string end) {
  if (end.size() > file_text.size()) {
    return false;
  }
  return file_text.compare(file_text.size() - end.size(), end.size(), end) == 0;
}

std::string removeEOF(std::string file_text, std::string end) {
  size_t pos = file_text.find(end);

  if (pos != std::string::npos) {
    file_text.erase(pos, end.length());
  }
  return file_text;
}

void new_file() {
  std::string file_name;
  std::string file_text;
  std::string end = "<EOF>";

  std::cout << "File Name:\n";
  std::getline(std::cin, file_name);

  std::cout << "Text end with \"<EOF>\"\n";
  std::getline(std::cin, file_text);

  bool correct_end = endsWith(file_text, end);
  if (correct_end) {
    std::string file = file_name + ".txt";

    int outfile = syscall(SYS_open, file.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (outfile == -1) {
      perror("Error opening File!");
      return;
    }

    std::string content = removeEOF(file_text, end);
    if (syscall(SYS_write, outfile, content.c_str(), content.size()) == -1) {
      perror("Error writing to file!");
      syscall(SYS_close, outfile);
      return;
    }
    syscall(SYS_close, outfile);
    std::cout << "Uploaded!\n";
  } else {
    std::cout << "Error, text does not end with <EOF>!\n";
  }
}

void list() {
  pid_t pid = syscall(SYS_fork);

  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    execl("/bin/ls", "ls", "-l", NULL);
    perror("execl");
    exit(EXIT_FAILURE);
  } else {
    int status;
    pid_t child_pid;

    while ((child_pid = syscall(SYS_waitid, -1, &status, 0)) > 0) {
      if (WIFEXITED(status)) {
        std::cout << "Child process exited with status: " << WEXITSTATUS(status)
                  << std::endl;
      } else {
        std::cout << "Child process exited abnormally" << std::endl;
      }
    }
  }
}

void find() {
  // deze vraagt om een string, en spawnt vervolgens twee nieuwe processen,
  // (find .) en (grep string). De processen worden met een pipe verbonden zodat
  // grep de output van find filtert alvorens deze te printen. Gebruik hiervoor
  // de system calls fork, exec, wait, dup2, close en pipe.
  std::string to_search;
  std::cout << "what to search?\n";
  std::getline(std::cin, to_search);

  int pipe_fd[2];
  if (syscall(SYS_pipe, pipe_fd) == -1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  pid_t pid = syscall(SYS_fork);
  if (pid == -1){
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0){
    syscall(SYS_close, pipe_fd[0]);
    syscall(SYS_dup2, pipe_fd[2], STDOUT_FILENO);
    syscall(SYS_close, pipe_fd[1]);
    execl("/usr/bin/find", "find", ".", NULL);
    perror("execl find");
    exit(EXIT_FAILURE);
  }

  pid = syscall(SYS_fork);
  if (pid == -1){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  
  if( pid == 0){
    syscall(SYS_close, pipe_fd[1]);
    syscall(SYS_dup2, pipe_fd[2], STDIN_FILENO);
    syscall(SYS_close, pipe_fd[0]);
    execl("/bin/grep", "grep", to_search.c_str(), NULL);
    perror("execl grep");
    exit(EXIT_FAILURE);
  }

  syscall(SYS_close, pipe_fd[0]);
  syscall(SYS_close, pipe_fd[1]);

  int status;
  while (syscall(SYS_waitid, &status) > 0){
    if (WIFEXITED(status)){
      std::cout << "Child exited with code: " << WIFEXITED(status) << std::endl;
    }
    else{
      std::cout << "Child exited abnormally" << std::endl;
    }
  }

}

void seek() // ToDo: Implementeer volgens specificatie.
{
  // Maakt twee bestanden, seek en loop.
  // In beide bestanden wordt een x geschreven, gevolgd door 5 megabyte aan lege
  // ruimte, gevolgd door nog een x. In het bestand seek gebeurt dit door
  // gebruik te maken van de system-call LSEEK,
  //    waarmee de offset van de file descriptor van een open bestand naar voren
  //    wordt geplaatst.
  // Voor het bestand loop wordt ~5 miljoen keer een "\0" karakter geschreven
  // met een for loop. Vergelijk de snelheid van beide bestanden, en kijk met ls
  // -lh (leesbare file size) en ls -lS
  //    (file size in daadwerkelijke blokken) naar beide bestanden.
  // Als het goed is hebben beide bestanden volgens ls -lh hetzelfde formaat,
  // maar niet volgens ls -lS

  const char *seek_filename = "seek.txt";
  const char *loop_filename = "loop.txt";
  char x = 'x';
  off_t offset = 5 * 1024 * 1024; //5mb

  int seek_fd = syscall(SYS_open, seek_filename, O_CREAT | O_WRONLY, 0644);
  if (seek_fd == -1){
    perror("Error opening seek file");
    return;
  }

  syscall(SYS_write, seek_fd, &x, 1);
  syscall(SYS_lseek, seek_fd, offset, SEEK_CUR);
  syscall(SYS_write, seek_fd, &x, 1);
  syscall(SYS_close, seek_fd);

  int loop_fd = syscall(SYS_open, loop_filename, O_CREAT | O_WRONLY, 0644);
   if (loop_fd == -1){
    perror("Error opening loop file");
    return;
  }

  syscall(SYS_write, loop_fd, &x, 1);
  syscall(SYS_close, loop_fd);

  std::cout << "Files 'Seek.txt' and 'loop.txt' have been created." << std::endl;
}

void src() // Voorbeeld: Gebruikt SYS_open en SYS_read om de source van de shell
           // (shell.cc) te printen.
{
  int fd = syscall(SYS_open, "shell.cc", O_RDONLY,
                   0755); // Gebruik de SYS_open call om een bestand te openen.
  char byte[1]; // 0755 zorgt dat het bestand de juiste rechten krijgt (leesbaar
                // is).
  while (
      syscall(SYS_read, fd, byte,
              1)) // Blijf SYS_read herhalen tot het bestand geheel gelezen is,
    std::cout << byte;
} //   zet de gelezen byte in "byte" zodat deze geschreven kan worden.
