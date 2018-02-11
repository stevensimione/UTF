#include "debug.h"
#include "wrappers.h"
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  lseek(SEEK_SET, program_state->bom_length, infile); /* Discard BOM */
  if(isDiff(program_state->in_file, program_state->out_file) == -1){
    free(program_state);
    close(outfile);
    close(infile);
    USAGE(argv[0]);
    return EXIT_FAILURE;
  }

  if(get_encoding_function()(infile, outfile) == -1){
    free(program_state);
    close(outfile);
    close(infile);
    USAGE(argv[0]);
    return EXIT_FAILURE;
  }
  if(program_state != NULL) {
    free(program_state);
  }
  //I think this is how this works
  close(outfile);
  close(infile);
  return EXIT_SUCCESS;
}
