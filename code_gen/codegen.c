#include <stdio.h>
#include <string.h>

#include "codegen.h"

FILE *ofp;

int stack_growth = 0;

int arg_counter = 0;
int sp = 0;

int label_counter = 1;
char scope[255] = "program";
char old_scope[255];
int scope_iter = 0;
int PRINT = 0;

Node *func_def;

int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./codegen <input file> <output file>\n");
        return 1;
    }

    // input and output file descriptors
    FILE *ifp;
    
    if((ifp = fopen(argv[1], "r")) == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    if((ofp = fopen(argv[2], "w+")) == NULL){
        printf("Error preparing output file\n");
        return 1;
    }


    // get abstract syntax tree from parser
    Node * AST = parse(argc, argv);

    printf("\nprinting symbol table:\n");
    print_symboltable();
    printf("\n");

    // start code gen
    if(AST != NULL) codegen(AST);

    standard_final();

    printf("\n\n-- exiting codegen normally\n\n");
    return 1;
}

int codegen(Node *AST){
    if(AST == NULL)
        return 0;


    //printf("AST->ID == %s\n", AST->ID);

    if(AST->root == 1){
        fprintf(ofp, ".data\n");
        print_scopeVars(ofp, "program");
        fprintf(ofp, "\n.text\n");
    }


    if(AST->funcDecl == 1){
        printf("-- funDecl: ");
        
        if(AST->children[1] == NULL)
            return 1;

        strcpy(old_scope, scope);

        strcpy(scope, AST->children[1]->ID);

        //rintf("scope: %s\n", scope);
        
        
        printf("%s:\n", AST->children[1]->ID); // func name:
        fprintf(ofp, "\n%s:\n", AST->children[1]->ID);


        fprintf(ofp, "move $fp $sp\n");

        push_ret_addr();

        codegen(AST->children[2]);
        codegen(AST->children[3]);

        int num_args = 0;
        Node *args = AST->children[2];
        while(args->children[num_args] != NULL){
            num_args++;
        }
        if(num_args != 0)
            num_args--;

        //printf("%s num_args: %d\n", AST->children[1]->ID, num_args);

        
        fprintf(ofp, "end_%s:", AST->children[1]->ID);


        pop_ret_addr(num_args);
        
        // if not main pop fp
        if(strcmp(AST->children[1]->ID, "main")){
            fprintf(ofp, "lw $fp 0($sp)\n");
        }
        
        fprintf(ofp, "jr $ra\n");

        strcpy(scope, old_scope);

        return 1;
    }

    if(AST->callStmt == 1){
        printf("callStmt: node->children[0]->ID == %s\n", AST->children[0]->ID);
        
        if(!strcmp(AST->children[0]->ID, "var")){

            Node *varSym = AST->children[0]->children[0]; 
            

            printf("varSym: %s\n", varSym->ID);

            //fprintf(ofp, "jal %s\n", AST->children[1]->children[0]->ID);

            codegen(AST->children[1]);

            fprintf(ofp, "sw $v0, %d($fp)\n", 4*get_pos(varSym->ID));
            return 1;
        }

        // if call args, process them
        if(AST->children[1] != NULL){
            int i = 0;
            //get number of args
            while(AST->children[1]->children[i] != NULL){
                i++;
            }
            i--;
            //fprintf(ofp, "sw $fp 0($sp)\naddiu $sp, $sp, -4\n");

            while(i>=0){
                if(!strcmp(AST->children[1]->children[i]->ID, "call")){
                    codegen(AST->children[1]->children[i]);
                    if(!strcmp(AST->children[0]->ID, "output"))
                        fprintf(ofp, "sw $v0, 0($sp)\naddiu $sp, $sp, -4\n");
                }
                else if(AST->children[1]->children[i]->op){
                    codegen(AST->children[1]->children[i]);
                }
                else if(AST->children[1]->children[i]->varID){
                    Node *id = AST->children[1]->children[i]->right;
                    fprintf(ofp, "lw $a0, %d($fp)\nsw $a0, 0($sp)\naddiu $sp, $sp, -4\n", 4*get_pos(id->ID));
                }
                else{
                    fprintf(ofp, "li $a0, %s\nsw $a0, 0($sp)\naddiu $sp, $sp, -4\n", AST->children[1]->children[i]->ID);
                }
                i--;
            }
        }



        fprintf(ofp, "jal %s\n", AST->children[0]->ID);



        return 1;
    }

    if(AST->op == 1){
        printf("-- op: %s\n", AST->ID);
        post_order_traverse(AST);
        return 1;
    }

    if(AST->params == 1){
        int arg_counter = 1;
        int i = 0;
        while(AST->children[i] != NULL){
            set_pos(AST->children[i]->children[1]->ID, arg_counter);
            arg_counter++;

            //printf("pos of %s == %d\n", AST->children[i]->children[1]->ID, get_pos(AST->children[i]->children[1]->ID));
            
            i++;
        }

        return 1;
    }

    if(AST->varDecl == 1){
        static int varcount = -1;
        printf("--varDecl: %s\n", AST->children[1]->ID);

        //make space for variable on stack
        fprintf(ofp, "addiu $sp, $sp, -4 #%s\n", AST->children[1]->ID);
        stack_growth++;
        
        set_pos(AST->children[1]->ID, varcount);
        varcount--;

        //printf("pos of %s == %d\n", AST->children[1]->ID, get_pos(AST->children[1]->ID));
        
        return 1;
    }

    if(AST->controlStmt){
        printf("--control statment: %s\n", AST->ID);

        //comp symbol
        printf("AST->children[0]->ID == %s\n", AST->children[0]->ID);
        char *comp_symbol = AST->children[0]->ID;

        //e1
        printf("e1: %s\n", AST->children[0]->left->right->ID);
        if(!strcmp(AST->children[0]->left->ID, "var")){
            fprintf(ofp, "lw $a0, %d($fp) #x\n", 4*get_pos(AST->children[0]->left->right->ID));
        }

        if(!strcmp(AST->ID, "selection-stmt") || !strcmp(AST->ID, "iteration-stmt")){
            fprintf(ofp, "sw $a0, 0($sp)\naddiu $sp $sp -4\n");
        
            //e2
            printf("AST->children[1]->ID: %s\n", AST->children[0]->right->ID);
            if(AST->children[0]->right->num)
                fprintf(ofp, "li $a0, %s\n", AST->children[0]->right->ID);
            else
                fprintf(ofp, "li $a0, %d\n", get_pos(AST->children[0]->right->ID));
            
        }
        

        

        if(!strcmp(AST->ID, "selection-stmt")){
            fprintf(ofp, "lw $t1, 4($sp)\naddiu $sp $sp 4\n");
        }

        if(!strcmp(AST->ID, "iteration-stmt")){

            fprintf(ofp, "loop:\nlw $t1, %d($fp)\nli $a0, 0\n", 4*get_pos(AST->children[0]->left->right->ID));
            if(!strcmp(comp_symbol, ">")){
                fprintf(ofp, "beq $t1 $a0 end_loop\n");
            }

            //cgen true
            if(AST->children[1] != NULL)
                codegen(AST->children[1]);

            
            fprintf(ofp, "j loop\n");
            fprintf(ofp, "end_loop:\n");

            sp+=1;

        }



        if(!strcmp(AST->ID, "selection-stmt")){

            if(!strcmp(comp_symbol, "==")){
                fprintf(ofp, "beq $a0 $t1 true_branch\n");
            }


            fprintf(ofp, "false_branch:\nb end_if\n");
            
            //cgen false branch
            if(AST->children[2] != NULL)
                codegen(AST->children[2]);

            fprintf(ofp, "true_branch:\n");

            // cgen true bracnh
            codegen(AST->children[1]);

            fprintf(ofp, "b end_%s\n", scope);

            fprintf(ofp, "end_if:\n");
        }


        return 1;
    }
    

    if(AST->returnStmt){
        printf("--return-stmt\n");
        if(AST->children[0]->num){
            fprintf(ofp, "li $v0, %s\n", AST->children[0]->ID);
        }
        else if(AST->children[0]->op){
            printf("!!! op\n");
            post_order_traverse(AST->children[0]);
            fprintf(ofp, "move $v0, $a0\n");
            //fprintf(ofp, "jal end_%s\n", scope);
        }
        return 1;
    }

    if(AST->compStmt){
        //printf("--compStmt\n");
    }

    if(AST->num == 1){
        //printf("num: %s\n", AST->ID);
    }

    if(AST->SYM){
        printf("--SYM: %s\n", AST->ID); 
    }

    int i = 0;
    while(AST->children[i] != NULL){
        codegen(AST->children[i]);
        i++;
    }

    return 1;
}

void push_ret_addr(){

    fprintf(ofp, "sw $ra, %d($sp)\naddiu $sp, $sp, -%d\n", (sp)*4, (sp+1)*4);
}

void pop_ret_addr(int num_args){
  
    printf("stack_growth: %d    num_args: %d      sp: %d\n", stack_growth, num_args, sp);

    fprintf(ofp, "\nlw $ra, %d($sp)\naddiu $sp, $sp, %d\n", 4*(1+stack_growth + sp), (num_args+stack_growth + 2 + sp)*4);
    stack_growth = 0;
    num_args = 0;
}

void post_order_traverse(Node *node){
    if(node == NULL)
        return;
    
    static int var_flag = 0;
    static int var_pos = 0;
    static int nested = 0;

    if(node->ID[0] == '='){
        printf("assign to %s\n", node->left->right->ID);
        var_flag = 1;
        var_pos = get_pos(node->left->right->ID);
    }

    nested++;
    post_order_traverse(node->left);

    post_order_traverse(node->right);
    nested--;

    printf("POT: node->ID == %s\n", node->ID);

   
    if(node->num){
        // load num into accumulator
        fprintf(ofp, "li $a0, %s\n", node->ID);

        // if there are further operations we must push onto stack
        if(node->leftNode){
            fprintf(ofp, "sw $a0, 0($sp)\naddiu $sp, $sp, -4\n");
        }
        else{
            if(!var_flag)
                fprintf(ofp, "lw $t1, 4($sp)\naddiu $sp, $sp, 4\n");
        }
    }
    else if(node->ID[0] == '+' || node->ID[0] == '-'){
        if(node->left->num)
            fprintf(ofp, "add $a0, $t1, $a0\nsw $a0, 0($sp)\naddiu $sp, $sp, -4\n");
        else{
            if(var_flag && node->left->SYM && node->right->SYM && nested <= 1){
                fprintf(ofp, "lw $a0, %d($fp)\nlw $t1, %d($fp)\n", 4*get_pos(node->left->right->ID), 4*get_pos(node->right->right->ID));
                if(node->ID[0] == '+')
                    fprintf(ofp, "add $a0, $t1, $a0\n");
                else
                    fprintf(ofp, "sub $a0, $t1, $a0\n");

                fprintf(ofp, "sw $a0, %d($fp)\n",  4*var_pos);
                fprintf(ofp, "addiu $sp, $sp, 4\n");
                var_flag = 0;
                var_pos = 0;
            }
            else if(var_flag && node->left->SYM && node->right->num && nested <= 1){
                fprintf(ofp, "lw $t1, %d($fp)\n", 4*get_pos(node->left->right->ID));
                if(node->ID[0] == '+')
                    fprintf(ofp, "add $a0, $t1, $a0\n");
                else
                    fprintf(ofp, "sub $a0, $t1, $a0\n");


                fprintf(ofp, "sw $a0, %d($fp)\n", 4*var_pos);
                //fprintf(ofp, "addiu $sp, $sp, 4\n");
                var_flag = 0;
                var_pos = 0;
            }
            else if(node->right->op){
                printf("  right node op\n");
            }
            else if(node->left->op){
                fprintf(ofp, "lw $a0, 4($sp)\nlw $t1 %d($fp)\n", 4*get_pos(node->left->right->ID));
                if(node->ID[0] = '+')
                    fprintf(ofp, "add $a0, $a0, $t1\n");
                else
                    fprintf(ofp, "sub $a0, $a0, $t1\n");

                fprintf(ofp, "sw $a0 0($sp)\naddiu $sp, $sp, -4\n");
                
            }
            else{
                fprintf(ofp, "lw $a0, %d($fp)\nlw $t1, %d($fp)\n", 4*get_pos(node->left->right->ID), 4*get_pos(node->right->right->ID));
                
                if(node->ID[0] = '+')
                    fprintf(ofp, "add $a0, $t1, $a0\n");
                else
                    fprintf(ofp, "sub $a0, $t1, $a0\n");
                
                fprintf(ofp, "sw $a0, 0($sp)\naddiu $sp, $sp, -4\n"); 
            }
        }
        
    }
    else if(node->ID[0] == '*'){
        if(var_flag && node->left->SYM && node->right->SYM && nested <= 1){
            fprintf(ofp, "lw $a0, %d($fp)\nlw $t1, %d($fp)\nmult $a0, $t1\nmflo $a0\nsw $a0, %d($fp)\n", 4*get_pos(node->left->right->ID), 4*get_pos(node->right->right->ID), 4*var_pos);
            //fprintf(ofp, "addiu $sp, $sp, 4\n");
            var_flag = 0;
            var_pos = 0;
        }
        else if(node->right->op){
            fprintf(ofp, "lw $a0, 4($sp)\nlw $t1 %d($fp)\nmult $a0, $t1\nmflo $a0\naddiu $sp, $sp, 4\n", 4*get_pos(node->left->right->ID));
            if(var_flag)
                fprintf(ofp, "sw $a0, %d($fp)\n", 4*var_pos);

        }
        else
            fprintf(ofp, "lw $a0, %d($fp)\nlw $t1, %d($fp)\nmult $a0, $t1\nmflo $a0\nsw $a0, 0($sp)\naddiu $sp, $sp, -4\n", 4*get_pos(node->left->right->ID), 4*get_pos(node->right->right->ID));

        
    }
    else if(node->ID[0] == '/'){
        fprintf(ofp, "lw $a0, %d($fp)\ndiv $a0, $t1\nsw $a0, 0($sp)\naddiu $sp, $sp, -4\n", 4*get_pos(node->right->right->ID));
    }
    else if(node->ID[0] == '='){
        if(node->left->SYM && node->right->SYM){
            fprintf(ofp, "la $t0, %d($fp)\nsw $t0, %d($fp)\n", 4*get_pos(node->right->right->ID), 4*get_pos(node->left->right->ID));
        }
        if(node->opRoot){
            if(node->right->num)
                fprintf(ofp, "sw $a0, %d($fp)\n", 4*get_pos(node->left->right->ID));
        }
    }
    else if(node->ID[0] == '<'){
        if(node->left->SYM && node->right->SYM){
            fprintf(ofp, "la $a0, %d($fp)\nla $t1, %d($fp)\nslt $a0, $a0, $t1\n", 4*get_pos(node->right->right->ID), 4*get_pos(node->left->right->ID));
            fprintf(ofp, "sw $a0, 0($sp)\naddiu $sp, $sp, -4\n");
        }
    }
    else if(!strcmp(node->ID,"<=")){
        if(node->left->SYM && node->right->SYM){
            fprintf(ofp, "li $a0, 0\n");
            fprintf(ofp, "la $a0, %d($fp)\nla $t1, %d($fp)\nble $a0, $a0, $t1\n", 4*get_pos(node->right->right->ID), 4*get_pos(node->left->right->ID));
            fprintf(ofp, "sw $a0, 0($sp)\naddiu $sp, $sp, -4\n");
        }
    }

}

void standard_final(){
    fprintf(ofp, "\n\n.text\n.globl __crt_out_of_bounds\n.globl output\n.globl input\n\n");
    fprintf(ofp, "__crt_out_of_bounds:\nli $v0, 10 # exit\nsyscall\n");
    fprintf(ofp, "\noutput:\nlw $a0, 4($sp)\nli $v0, 1\nsyscall\nli $v0, 11\nli $a0, 0x0a\nsyscall\naddiu $sp, $sp, 4\nli $a0, 0\nj $ra\n");
    fprintf(ofp, "\ninput:\nli $v0, 5\nsyscall\njr $ra\n\n");
}
