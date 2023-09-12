/*
    Name 1: Paul Han
    UTEID 1: pjh2235
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} destination_register;

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} source_register1;

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} source_register2;

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} base_register;

//arrays containing the pre-defined register structs
//TODO: Is there an easier way to do this without hard coding?
const destination_register arr_destination_register[] = {
	{"r0",0b0000000000000000},{"r1",0b0000001000000000},{"r2",0b0000010000000000},{"r3",0b0000011000000000},
	{"r4",0b0000100000000000},{"r5",0b0000101000000000},{"r6",0b0000110000000000},{"r7",0b0000111000000000}
};

const source_register1 arr_source_register1[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000001000000},{"r2",0b0000000010000000},{"r3",0b0000000011000000},
	{"r4",0b0000000100000000},{"r5",0b0000000101000000},{"r6",0b0000000110000000},{"r7",0b0000000111000000}
};

const source_register2 arr_source_register2[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000000000001},{"r2",0b0000000000000010},{"r3",0b0000000000000011},
	{"r4",0b0000000000000100},{"r5",0b0000000000000101},{"r6",0b0000000000000110},{"r7",0b0000000000000111}
};

const base_register arr_base_register[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000001000000},{"r2",0b0000000010000000},{"r3",0b0000000011000000},
	{"r4",0b0000000100000000},{"r5",0b0000000101000000},{"r6",0b0000000110000000},{"r7",0b0000000111000000},
};

typedef struct{	
	char opcode[6];
	unsigned short machine_code;
	int num_registers;
	int base_register;
	__int16_t num_bits_offset_immediate;
} opcode;

const opcode arr_opcode[] = {
	{"lea",0b1110000000000000, 1, FALSE, 9},
  {"brn",0b0000100000000000, 0, FALSE, 9},
  {"brz",0b0000010000000000, 0, FALSE, 9},
  {"brp",0b0000001000000000, 0, FALSE, 9},
  {"brnz",0b0000110000000000, 0, FALSE, 9},
	{"brzp",0b0000011000000000, 0, FALSE, 9},
  {"brnp",0b0000101000000000, 0, FALSE, 9},
  {"br",0b0000111000000000, 0, FALSE, 9},
  {"brnzp",0b0000111000000000, 0, FALSE, 9},
	{"add",0b0001000000000000, 3, FALSE, 0},
  {"add",0b0001000000100000, 2, FALSE, 5},		
	{"and",0b0101000000000000, 3, FALSE, 0},
  {"and",0b0101000000100000, 2, FALSE, 5},	
	{"jmp",0b1100000000000000, 1, TRUE, 0},	
	{"jsr",0b0100100000000000, 0, FALSE, 11},
  {"jsrr",0b0100000000000000, 1, TRUE, 0},	
	{"ldb",0b0010000000000000, 2, TRUE, 6},
  {"ldw",0b0110000000000000, 2, TRUE, 6},	
	{"not",0b1001000000111111, 2, FALSE, 0},	
	{"ret",0b1100000111000000, 0, TRUE, 0},	
	{"rti",0b1000000000000000, 0, FALSE, 0},	
	{"lshf",0b1101000000000000, 2, FALSE, 4},
	{"rshfl",0b1101000000010000, 2, FALSE, 4},
	{"rshfa",0b1101000000110000, 2, FALSE, 4},
	{"stb",0b0011000000000000, 2, TRUE, 6},	
	{"stw",0b0111000000000000, 2, TRUE, 6},
	{"trap",0b1111000000000000, 0, FALSE, 8},
	{"xor",0b1001000000000000, 3, FALSE, 0},
	{"xor",0b1001000000100000, 2, FALSE, 5},	
	{"nop",0b0000000000000000, 0, FALSE, 0},	
	{"halt",0b1111000000100101, 0, FALSE, 0}
};

#define NUM_INSTRUCTIONS 31

/* return 16 bit instruction in machine code */
__uint16_t fetch_instruction(void);
/* 
* Function declarations
*/
/* decode instructions, return opcode index into array */
//Do I really have to have 31 case and switch or if statements for each opcode...
int decode_instruction(__uint16_t instruction);
void execute_instruction(int opcode_index, __uint16_t instruction);
int type_base_register(__uint16_t instruction);
int type_destination_register(__uint16_t instruction);
int type_source1_register(__uint16_t instruction);
int type_source2_register(__uint16_t instruction);
short value_immediate(__uint16_t instruction, int num_bits);

/* Function definitions */
void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */    
  __uint16_t instruction = fetch_instruction(); 
  int index = decode_instruction(instruction); 
  execute_instruction(index, instruction);
}

/* 
  Load the LSB and the MSB from memory using the address of the PC right shifted by 1.
  Since memory locations in LC3-b are half-word addressable (word being 32 bits) while modern architectures are
  32 bits, each memory location in the simulator contains TWO instructions. Therefore, we must right shift or 
  divide the address by 2 to obtain the correct address.
  Left shift MSB by 8 bits, OR the instruction with the LSB and MSB to obtain 16 bit instruction
*/
__uint16_t fetch_instruction(void){
  __uint16_t instruction = 0;
  __uint16_t least_byte = MEMORY[CURRENT_LATCHES.PC >> 1][0];
  __uint16_t most_byte = MEMORY[CURRENT_LATCHES.PC >> 1][1];
  instruction |= least_byte;
  most_byte = most_byte << 8;
  instruction |= most_byte;
  return instruction;
}

/* 
    Compare the opcode with the machine code embedded into the const opcode array 
    If there is a match, return the index of the opcode within the const opcode array
*/
//Im so sorry to anyone who has to see this code
int decode_instruction(__uint16_t instruction){
  __uint16_t opcode = instruction & 0xF000; 
  if(opcode == 0x0000){
    opcode = instruction & 0xFE00;
  }else{
    opcode = instruction & 0xF000;
  }
  int index = 0;
  for(int i = 0; i < NUM_INSTRUCTIONS; i++){
    if(opcode == arr_opcode[i].machine_code){
      //ADD
      if(opcode == 0x1000 && (instruction & 0xF020) == 0x1000){
        index = i;
      }else if(opcode == 0x1000 && (instruction & 0xF020) == 0x1020){
        i++;
        index = i;
      }
      //JSR, JSRR
      if(opcode == 0x4000 && (instruction & 0xF800) == 0x4800){
        index = i;
      }
      else if(opcode == 0x4000 && (instruction & 0xF800) == 0x4000){
        index = i;
      }
      if(opcode == 0x5000 && (instruction & 0xF020) == 0x5020){
        i++;
        index = i;
      }else if(opcode == 0x5000 && (instruction & 0xF020) == 0x5000){
        index = i;
      }
      //RET
      if(opcode == 0xC000 && (instruction & 0xF1C0) == 0xC1C0 && arr_opcode[i].machine_code == 0xC1C0){
        index = i;
      }
      else if(opcode == 0xC000 && (instruction & 0xF1C0) < 0xC1C0 && arr_opcode[i].machine_code < 0xC1C0){
        index = i;
      }
      //xor
      if(opcode == 0x9000 && (instruction & 0xF020) == 0x9000){
        index = i;
      }else if(opcode == 0x9000 && (instruction & 0xF020) == 0x9020){
        i++;
        index = i;
      }
      // shift instructions
      if(opcode == 0xD000 && (instruction & 0xF030) == 0xD000){
        index = i;
      }else if(opcode == 0xD000 && (instruction & 0xF030) == 0xD010){
        i++;
        index = i;
      }
      else if(opcode == 0xD000 && (instruction & 0xF030) == 0xD030){
        i += 2;
        index = i;
      }
      index = i;
    }
  }
  return index;
}

void execute_instruction(int opcode_index, __uint16_t instruction){
  int num_registers = arr_opcode[opcode_index].num_registers;
  int base_register = arr_opcode[opcode_index].base_register;
  int num_bits_offset = arr_opcode[opcode_index].num_bits_offset_immediate;
  const char *opcode = arr_opcode[opcode_index].opcode;
  switch(num_registers){
    case 0:{
      if(num_bits_offset == 0){
        if(strcmp(opcode,"nop")==0){
          NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
          break;
        }
        if(strcmp(opcode,"ret")==0){
          NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[7];
          break;
        }
        if(strcmp(opcode,"rti")==0){
          NEXT_LATCHES.PC = MEMORY[CURRENT_LATCHES.REGS[6]>>1];
          CURRENT_LATCHES.REGS[6] += 2;
          int temp = MEMORY[CURRENT_LATCHES.REGS[6]>>1];
          CURRENT_LATCHES.REGS[6] += 2;
          temp = Low16bits(temp);
          CURRENT_LATCHES.N = (temp & 0x0004) >> 2;
          CURRENT_LATCHES.Z = (temp & 0x0002) >> 1;
          CURRENT_LATCHES.P = (temp & 0x0001);
          break;
        }
      }
    }
    case 1:{
      
      break;
    }
    case 2:{
      int base_source_reg_index = type_base_register(instruction);
      int dest_reg_index = type_destination_register(instruction);
      unsigned short dest_reg = CURRENT_LATCHES.REGS[dest_reg_index];
      unsigned short base_source_reg = CURRENT_LATCHES.REGS[base_source_reg_index];
       
      if(strcmp(opcode,"add")==0){
        dest_reg = Low16bits(dest_reg);
        base_source_reg = Low16bits(base_source_reg);
        __int16_t value = value_immediate(instruction, num_bits_offset); 
        dest_reg = Low16bits(value + base_source_reg);
        if(dest_reg == 0){
          NEXT_LATCHES.Z = 1;
        }
        else if(dest_reg < 0){
          NEXT_LATCHES.N = 1;
        }
        else{
          NEXT_LATCHES.P = 1;
        }
        NEXT_LATCHES.REGS[dest_reg_index] = dest_reg;
        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
      } 
      break;
    }
    case 3:{
      break;
    }
  }
}

int type_base_register(__uint16_t instruction){
  unsigned short bit_mask = 0x0E00;
  for(int i = 0; i < NUM_INSTRUCTIONS; i++){
    if(instruction & bit_mask == arr_base_register[i].machine_code){
      return i;
    }
  }
}

int type_destination_register(__uint16_t instruction){
  unsigned short bit_mask = 0x0E00;
  for(int i = 0; i < NUM_INSTRUCTIONS; i++){
    if(instruction & bit_mask == arr_destination_register[i].machine_code){
      return i;
    }
  }
}

int type_source1_register(__uint16_t instruction){
  unsigned short bit_mask = 0x01C0;
  for(int i = 0; i < NUM_INSTRUCTIONS; i++){
    if(instruction & bit_mask == arr_source_register1[i].machine_code){
      return i;
    }
  }
}

int type_source2_register(__uint16_t instruction){
  unsigned short bit_mask = 0x0007;
  for(int i = 0; i < NUM_INSTRUCTIONS; i++){
    if(instruction & bit_mask == arr_source_register2[i].machine_code){
      return i;
    }
  }
}
short value_immediate(__uint16_t instruction, int num_bits){
  __int16_t immediate_value = instruction;
  __uint16_t bit_mask = 0x8000;
  for(int i = 0; i < 16 - num_bits; i++){
    bit_mask = bit_mask >> 1;
  }
  immediate_value = immediate_value & bit_mask;
  for(int i = 0; i < num_bits - 1; i++){
    immediate_value = immediate_value >> 1; 
  }
  if(immediate_value == 1){
    //sign extend
    immediate_value = instruction; 
    bit_mask = 0x8000;
    for(int i = 0; i < 16 - num_bits; i++){
      immediate_value = immediate_value | bit_mask;
      bit_mask = bit_mask >> 1;
    }
  }
  else{
    //zero extend
    immediate_value = instruction;
    bit_mask = 0x7FFF;
    for(int i = 0; i < 16 - num_bits; i++){
      immediate_value = immediate_value & bit_mask;
      bit_mask = bit_mask >> 1;
    }
  }

  return immediate_value;
}