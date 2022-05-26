#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 256

typedef struct variables{
    char *name;
    char *scope;
    char *type;
    int size;
    char *data_size;

    int num_lines_function;
    int num_var_function;

    int heap_stack; //0 if global, 1 if heap, 2 if stack, 3 if ROData

    struct variables *next;

}VARNode; 

//initliaze VARNode
VARNode *initialize_var(char *name, char *scope, char *type, int size, char *data_size, int lines, int variables, int heap_stack){
    VARNode *var = malloc(sizeof(VARNode));

    var->name = strdup(name);
    var->scope = strdup(scope);
    var->type = strdup(type);
    var->size = size;
    var->data_size = strdup(data_size);
    var->heap_stack = heap_stack;

    //for global variables, the following is just -1
    var->num_lines_function = lines;
    var->num_var_function = variables;

    var->next = NULL;

    return var;
}

//insert into VARNode linked list
VARNode *insert_var(VARNode *head, char *name, char *scope, char *type, int size, char *data_size, int lines, int variables, int heap_stack){
    VARNode *new_node = initialize_var(name, scope, type, size, data_size, lines, variables, heap_stack);

    if (head == NULL){
        head = new_node;
    }

    else{
        VARNode *temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }

        temp->next = new_node;
    }
    
    return head;
}

typedef struct functions{
    char *name;
    int total_lines;
    int total_variables;
    struct functions *next;
}FUNNode;

//intitalize FUNNode
FUNNode *intialize_fun(char *name, int total_lines, int total_variables){
    FUNNode *fun = malloc(sizeof(FUNNode));
    
    fun->name = strdup(name); //https://stackoverflow.com/questions/10162152/how-to-work-with-string-fields-in-a-c-struct

    fun->total_lines = total_lines;
    fun->total_variables = total_variables;
    fun->next = NULL;
    return fun;
}

//insert into FUNNode linked list
FUNNode *insert_fun(FUNNode *head, char *name, int lines, int variables){
    FUNNode *new_node = intialize_fun(name, lines, variables);

    if (head == NULL){
        head = new_node;
    }

    else{
        FUNNode *temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }

        temp->next = new_node;
    }

    return head;
}

//gets rid of any comments made in the line
char *cleanline(char *line){
    char *new_line = malloc(sizeof(char) * MAX_LENGTH);
    int index = 0;
    int i = 0;

    while(line[i] != '\0'){
        if (line[i] == '/' && line[i+1] == '/'){
            break;
        }

        new_line[index] = line[i];
        index++;
        i++;
    }

    new_line[index] = '\0';

    return new_line;
}

//returns the line with all the whitespace removed
char *remove_whitespace(char *line){
    char *new_line = malloc(sizeof(char) * MAX_LENGTH);
    int index = 0;
    int i = 0;

    while(line[i] != '\0'){
        if (line[i] != ' '){
            new_line[index] = line[i];
            index++;
        }
        i++;
    }

    new_line[index] = '\0';

    return new_line;
}

//use to count the number of lines in the program/function
//use "" as fun_name when counting lines of entire program
int line_counter(FILE *fp, char *path, char *fun_name){
    fp = fopen(path, "r");
    int counter = 0;
    char line[MAX_LENGTH];

    if (fun_name == ""){
        while (fgets(line, MAX_LENGTH, fp)){
            counter++;
        }
    }

    else{
        while (fgets(line, MAX_LENGTH, fp)){
            counter++;
        }
    }

    fclose(fp);
    return counter; 
}

//returns 1 if line is for loop, 2 if while loop, and 0 otherwise
int check_loop(char *line){
    char *temp;
    temp = strdup(line);

    temp = strtok(temp, " "); 
    while(temp != NULL) {
        if (strstr(remove_whitespace(temp), "for(")){
            return 1;
        }

        else if (strstr(remove_whitespace(temp), "while(")){
            return 2;
        }

        temp = strtok(NULL, " ");
    }

    return 0;
}

//returns 1 if line is if/else statement, and 0 otherwise
int check_if_else(char *line){
    char *temp;
    temp = strdup(line);

    temp = strtok(temp, " "); 
    while(temp != NULL) {

        if (strstr(remove_whitespace(temp), "if(") || strstr(remove_whitespace(temp), "else")){
            return 1;
        }
        temp = strtok(NULL, " ");
    }

    return 0;
}

//returns 1 if line is return or printf, and 0 otherwise
int check_return(char *line){
    char *temp;
    temp = strdup(line);

    temp = strtok(temp, " "); 
    while(temp != NULL) {

        if (strstr("return", remove_whitespace(temp)) || strstr(remove_whitespace(temp), "printf")){
            return 1;
        }
        temp = strtok(NULL, " ");
    }
    return 0;
}

//gets the number of variables in the line
int num_variables_line(char *line){
    int num_variables = 0;

    if (strstr(line, "alloc")){
        return 1;
    }

    if (strstr(line, "[") && strstr(line, "]")){
        return 1;
    }

    for(int i=0; i < strlen(line);i++)  
    {
    	if(line[i] == ',')
    	{
            //counts the number of commas in the line
            num_variables++;
		}
 	}

    num_variables += 1; //2 commas means we have 3 variables, 3 commas means 4 variables, and so on. 

 	return num_variables;
}

//returns 1 if line is a function header, 2 if a variable, 0 otherwise
int check_function_datatype(char *line){
    char* temp = strdup(line);

    int loop = check_loop(temp);
    int if_else = check_if_else(temp);
    int return_ = check_return(temp);

    const char *variables[15] = {"void", "int","float","char", "int*", "float*", "char*", "char**",
                            "int[]", "float[]", "char[]", "*int", "*float", "*char", "**char"};

    if (loop == 0 && if_else == 0 && return_ == 0){
        for (int i = 0; i < 15; i++){
            temp = remove_whitespace(temp);
            if (strstr(temp, variables[i])){
                if (strstr(temp, ";")){
                    return 2; 
                }
                return 1;
            }
        }
    }
    
    return 0;
}

//counts the number of variables in the header
int num_variables_header(char *function_header){
    int num_variables = 0;
    char *temp = strdup(function_header);
    temp = strtok(temp, " "); // temp is now the return type of the header
    temp = strtok(NULL, ")"); // temp is now containing all the paramaters of the header

    if (check_function_datatype(temp) == 1){//means header has atleast one function
        num_variables = num_variables_line(temp);
    }

    return num_variables;
}

//stops the line after encountering either a ";" or a "=" or a ")"
char *remove_equal_colon_bracket(char *line){
    char *new_line = malloc(sizeof(char) * MAX_LENGTH);
    int index = 0;
    int i = 0;
    
    while(line[i] != '\0'){
        if (line[i] == '=' || line[i] == ';' || line[i] == ')' || line[i] == '['){
            break;
        }
        
        new_line[index] = line[i];
        index++;        
        i++;
    }

    new_line[index] = '\0';

    return new_line;
}

//returns the line with all the colons/stars/whitespace removed
char *remove_stars(char *line){
    line = remove_equal_colon_bracket(line);
    line = remove_whitespace(line);
    char *new_line = malloc(sizeof(char) * MAX_LENGTH);
    int index = 0;
    int i = 0;

    while(line[i] != '\0'){
        if (line[i] != '*'){
            new_line[index] = line[i];
            index++;
        }
        i++;
    }

    new_line[index] = '\0';

    return new_line;
}

//removes the last bracket in line
char *remove_last_bracket(char *line){
    char *new_line = malloc(sizeof(char) * MAX_LENGTH);
    int index = 0;
    int i = 0;
    int counter = 0;
    
    while(line[i] != '\0'){
        if (line[i] == ')'){
            if (counter == 1){
                break;
            }
            counter++;
        }
        
        new_line[index] = line[i];
        index++;        
        i++;
    }

    new_line[index] = '\0';
    return new_line;
}

//returns the stuff inside brackets
//if option is 1, then malloc 
//option 2 is calloc, anything else is [] (for example, option = 3)
char *get_inside_bracket(char *line, int option){
    if (option == 1){
        char *datasize = malloc(sizeof(char) * MAX_LENGTH);
        int index = 0;
        int i = 0;

        while(line[i] != '\0'){
            if (line[i] == '('){
                i++;
                while(line[i] != ';'){
                    datasize[index] = line[i];
                    index++;
                    i++;
                } 
            }
            i++;
        }

        datasize[index-1] = '\0';

        return datasize;
    }

    else if(option == 2){
        char *datasize = malloc(sizeof(char) * MAX_LENGTH);
        int index = 0;
        int i = 0;

        while(line[i] != '\0'){
            if (line[i] == '('){
                i++;
                while(line[i] != ';'){
                    datasize[index] = line[i];
                    index++;
                    i++;
                } 
            }
            i++;
        }

        datasize[index-1] = '\0';

        i = 0;
        while(datasize[i] != '\0'){
            if (datasize[i] == ','){
                datasize[i] = '*';
            }

            i++;
        }

        return datasize;
    }

    else{
        char *datasize = malloc(sizeof(char) * MAX_LENGTH);
        int index = 0;
        int i = 0;

        while(line[i] != '\0'){
            if (line[i] == '['){
                i++;
                while(line[i] != ']' && line[i] != ' '){
                    datasize[index] = line[i];
                    index++;
                    i++;
                } 
            }
            i++;
        }

        datasize[index] = '\0';

        if (index == 0){
            return "INVALID";
        }

        return datasize;
    }

}

//returns the value inside the bracket, given that the bracket is not empty
int value_of_bracket(char *line){
    int i = 0;
    int counter = 0;
    while (line[i] != '\0'){
        if (line[i] == '{'){
            i++;
            while(line[i] != '}'){
                if(line[i] == ','){
                    counter++;
                }
                i++;
            }
        }
        i++;
    }

    counter += 1;
    return counter;
}

//returns the size of the datatype given in string format, in either int ot char* form 
char *get_sizeof(char *datatype, int *size){
    *size = 0;

    char *variable = strdup(datatype);

    if (strstr(variable, "malloc")){
        *size = -1;
        return get_inside_bracket(datatype, 1);
    }

    else if (strstr(variable, "calloc")){
        *size = -1;
        return get_inside_bracket(datatype, 2);
    }

    else if (strstr(variable, "void")){
        *size = sizeof(void);
        return "sizeof(void)";
    }

    else if (strstr(variable, "int")){
        if (strstr(variable, "*")){
            *size = sizeof(int*);
            return "sizeof(int*)";
        }

        else if (strstr(variable, "[")){
            *size = -1;
            
            if (strcmp(get_inside_bracket(datatype, 3), "INVALID") == 0){
                *size = value_of_bracket(datatype);
                return "INVALID";
            }

            else{
                return get_inside_bracket(datatype, 3);
            }
        }

        else{
            *size = sizeof(int);
            return "sizeof(int)";
        }
    }

    else if (strstr(variable, "float")){
        if (strstr(variable, "*")){
            *size = sizeof(float*);
            return "sizeof(float*)";
        }

        else if (strstr(variable, "[")){
           *size = -1;
            
            if (strcmp(get_inside_bracket(datatype, 3), "INVALID") == 0){
                *size = value_of_bracket(datatype);
                return "INVALID";
            }

            else{
                return get_inside_bracket(datatype, 3);
            }
        }

        else{
            *size = sizeof(float);
        }
    }

    else if (strstr(variable, "char")){
        if (strstr(variable, "**")){
            *size = sizeof(char**);
            return "sizeof(char**)";
        }

        else if (strstr(variable, "*")){
            *size = sizeof(char*);
            return "sizeof(char*)";
        }

        else if (strstr(variable, "[")){
             *size = -1;
            
            if (strcmp(get_inside_bracket(datatype, 3), "INVALID") == 0){
                *size = value_of_bracket(datatype);
                return "INVALID";
            }

            else{
                return get_inside_bracket(datatype, 3);
            }
        }

        else{
            *size = sizeof(char);
            return "sizeof(char)";
        }
    }
}

//return 1 if line contains malloc or calloc, return 2 otherwise
int is_heap_RO(char *line){
    if (strstr(line, "malloc(") || strstr(line, "calloc(")){
        return 1;
    }

    line = remove_whitespace(line);

    if (strstr(line, "char*") || strstr(line, "*char")){
        if (strstr(line, "=")){
            return 3;
        }
    }

    return 2;
}

//returns the datatype of the variable in the line
char *get_datatype(char *line){
    char *datatype = strdup(line);

    if (strstr(datatype, "void")){
        return "void";
    }

    else if (strstr(datatype, "int")){
        if (strstr(datatype, "*")){
            return "int*";
        }

        else if (strstr(datatype, "[")){
            return "int[]";
        }

        else{
            return "int";
        }
    }

    else if (strstr(datatype, "float")){

        if (strstr(datatype, "*")){
            return "float*";
        }

        else if (strstr(datatype, "[")){
           return "float[]";
        }

        else{
            return "float";
        }
    }

    else if (strstr(datatype, "char")){

        if (strstr(datatype, "**")){
            return "char**";
        }

        else if (strstr(datatype, "*")){
            return "char*";
        }

        else if (strstr(datatype, "[")){
           return "char[]";
        }

        else{
            return "char";
        }
    }
}

//finds the global variables of the program
//assumption is that global variables are all declared at the starting of the program
VARNode *find_global_var(FILE *fp, char *path, VARNode *head){
    char line[MAX_LENGTH];
    char temp_line[MAX_LENGTH];
    char *datatype = malloc(MAX_LENGTH*sizeof(char));

    char *data_size = malloc(MAX_LENGTH * sizeof(char));
    data_size = NULL;

    char *name;
    fp = fopen(path, "r");

    int check_heap = 0;
    
    while (fgets(line, MAX_LENGTH, fp)){
        if (strstr(line, "{") != NULL){ //start of a block, meaning we can't have any global variables afterwards
            break;
        }
    
        else{
            if (check_function_datatype(line) == 2 && check_function_datatype(remove_equal_colon_bracket(line)) == 1){
                int variables_declared = num_variables_line(line);

                char *var_name = NULL;
                char *datatype = malloc(MAX_LENGTH*sizeof(char));

                strcpy(temp_line, line);
                int num_var_declared = num_variables_line(temp_line);

                char *temp = strtok(temp_line, " "); //gets the datatype of the variable

                datatype = get_datatype(line);

                int size = 0;
                get_sizeof(line, &size);


                if (variables_declared > 1){
                    while(num_var_declared > 0){
                        check_heap = is_heap_RO(line);
                        if (num_var_declared == 1){
                            temp = strtok(NULL, ";");
                        }

                        else{
                            temp = strtok(NULL, ","); //gets the first variable name
                        }
                        temp = remove_equal_colon_bracket(temp);
                        var_name = strdup(temp);
                        var_name = remove_whitespace(var_name);
                        
                        head = insert_var(head, remove_stars(var_name), "global", datatype, size, get_sizeof(line, &size), -1, -1, check_heap);
                        num_var_declared--;
                    }

                }

                else{
                    check_heap = is_heap_RO(line);
                    temp = strtok(NULL, " "); //gets the variable name
                    temp = remove_equal_colon_bracket(temp); //gets rid of the "=" or ";" from the name of the variable.
                    var_name = strdup(temp);
                    var_name = remove_whitespace(var_name);

                    head = insert_var(head, remove_stars(var_name), "global", datatype, size, get_sizeof(line, &size), -1, -1, check_heap);
                }

            }
        }
    }

    fclose(fp);

    return head;
}

//gets the datatype of the header, given there is only one variable in the header.
char *get_header_datatype(char *function_header){
    char *temp = strdup(function_header);
    char *datatype = strtok(temp, " ");
    datatype = strtok(NULL, ")");

    return get_datatype(datatype);
}

//adds the header variables to the linked list
VARNode *print_header_variables(char *function_header, char *fun_name, int variables, VARNode *var_head){
    int counter = variables;
    char *datatype = malloc(sizeof(char) * MAX_LENGTH);; 
    char *var_name = malloc(sizeof(char) * MAX_LENGTH);;

    char *temp = strdup(function_header);
    char *head_name = strtok(temp, " ");
    head_name = strtok(NULL, "(");

    if (variables == 0){
        //no variables in the function header
        var_head = insert_var(var_head, "-invalid", fun_name, "-datatype", -5, "-datasize", -5, -5, -5);
    }

    else if (variables == 1){

        head_name = strtok(NULL, " ");

        head_name = strtok(NULL, ")");
        var_name = strdup(head_name);

        datatype = strdup(get_header_datatype(function_header));

        char *datasize = malloc(sizeof(char) * MAX_LENGTH);
        int index = 0;
        int i = 0;

        while(function_header[i] != '\0'){
            if (function_header[i] == ' '){
                i++;
                while(function_header[i] != ')'){
                    datasize[index] = function_header[i];
                    index++;
                    i++;
                } 
            }
            i++;
        }

        datasize[index] = ')';
        datasize[index+1] = '\0';

        int size = 0;
        get_sizeof(datasize, &size);
        var_head = insert_var(var_head, remove_stars(var_name), fun_name, datatype, size, get_sizeof(datasize, &size), -1, -1, 2);
    }

    else{
        while (counter > 0){

            head_name = strtok(NULL, ","); 
            char *var_temp = strdup(head_name);
            sscanf(var_temp, "%s %s", datatype, var_name); //https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm

            int size = 0;
            get_sizeof(var_temp, &size);

            var_head = insert_var(var_head, remove_stars(var_name), fun_name, get_datatype(head_name), size, get_sizeof(var_temp, &size), -1, -1, 2);

            counter--;
        }
    }

    return var_head;
}

//return 1 if line contains malloc or calloc
int check_alloc(char *line){
    if (strstr("alloc", remove_whitespace(line))){
            return 1;
    }

    return 0;
}

//return 1 if line contains ; and =
int check_equal_colon(char *line){
    if (strstr(line, "=") && strstr(line, ";")){
            return 1;
    }

    return 0;
}

//adds the variables that have had their values updated (originally global, or changed to malloc/calloc) 
//to the linked list
void add_updated_var(char* name, VARNode *var_head, VARNode *global_head, char *fun_name, int check_heap, char *line){
    VARNode *update_value = var_head;
    int size = update_value->size;
    char *datasize = strdup(update_value->data_size);

    if (strstr(line, "alloc")){
        datasize = strdup(get_sizeof(line, &size));

        while (update_value != NULL){
        if (strcmp(update_value->name, name) == 0){
            if (strstr(line, "alloc") != NULL || strcmp(update_value->scope, "global") == 0){
                if (check_heap != update_value->heap_stack){
                    var_head = insert_var(var_head, update_value->name, fun_name, update_value->type, 
                            size, datasize, -1, -1, check_heap);
                }
                
                break;
            }
        }
        update_value = update_value -> next;
        }

        update_value = global_head;

        while (update_value != NULL){
            if (strcmp(update_value->name, name) == 0){
                if (strstr(line, "alloc") != NULL || strcmp(update_value->scope, "global") == 0){
                    var_head = insert_var(var_head, update_value->name, fun_name, update_value->type, 
                                size, datasize, -1, -1, check_heap);
                    break;
                }
            }
            update_value = update_value -> next;
        }
    }

    else{
        while (update_value != NULL){
        if (strcmp(update_value->name, name) == 0){
            if (strstr(line, "alloc") != NULL || strcmp(update_value->scope, "global") == 0){
                if (check_heap != update_value->heap_stack){
                    var_head = insert_var(var_head, update_value->name, fun_name, update_value->type, 
                            update_value->size, update_value->data_size, -1, -1, check_heap);
                }
                
                break;
            }
        }
        update_value = update_value -> next;
        }

        update_value = global_head;

        while (update_value != NULL){
            if (strcmp(update_value->name, name) == 0){
                if (strstr(line, "alloc") != NULL || strcmp(update_value->scope, "global") == 0){
                    var_head = insert_var(var_head, update_value->name, fun_name, update_value->type, 
                                update_value->size, update_value->data_size, -1, -1, check_heap);
                    break;
                }
            }
            update_value = update_value -> next;
        }
    }

        
}

//goes through each function and adds the variables inside it to the linked list
VARNode *analyze_function(FILE *fp, char *function_header, VARNode *var_head, VARNode *global_head){
    char line[MAX_LENGTH];
    char temp_line[MAX_LENGTH];
    char prev_line[MAX_LENGTH];

    char temp_header[MAX_LENGTH];
    strcpy(temp_header, function_header);

    //get the name of the function
    char *fun_name = strtok(temp_header, " ");
    fun_name = strtok(NULL, "(");
    fun_name = remove_stars(fun_name);

    int num_variables = num_variables_header(function_header);
    int total_lines = 0;
    
    VARNode *temp_var_head = NULL;

    temp_var_head = print_header_variables(function_header, fun_name, num_variables, temp_var_head);

    int check_heap = 2;

    char *data_size = NULL;

    int blocker = 0;
    
    while (fgets(line, MAX_LENGTH, fp)){
        if (strstr(line, "{") != NULL && strstr(line, "}") == NULL){
            blocker += 1;
        }

        if (strstr(line, "}") != NULL && strstr(line, "{") == NULL){
            blocker -= 1;
            if (blocker < 0){
                break;
            }
        }

        total_lines++;
        if (check_function_datatype(line) == 2){
            if (check_function_datatype(remove_equal_colon_bracket(line)) == 1){ //basically checks if there is any datatype before the "=" sign
                int variables_declared = num_variables_line(line);

                char *var_name = NULL;
                char *datatype = malloc(MAX_LENGTH*sizeof(char));

                strcpy(temp_line, line);
                int num_var_declared = num_variables_line(temp_line);

                char *temp = strtok(temp_line, " "); //gets the datatype of the variable

                datatype = get_datatype(line);
                
                int size = 0;
                get_sizeof(line, &size);


                if (variables_declared > 1){
                    while(num_var_declared > 0){
                        check_heap = is_heap_RO(line);
                        if (num_var_declared == 1){
                            temp = strtok(NULL, ";");
                        }

                        else{
                            temp = strtok(NULL, ","); //gets the first variable name
                        }
                        temp = remove_equal_colon_bracket(temp);
                        var_name = strdup(temp);
                        var_name = remove_whitespace(var_name);
                        
                        temp_var_head = insert_var(temp_var_head, remove_stars(var_name), fun_name, datatype, size, get_sizeof(line, &size), -1, -1, check_heap);
                        num_var_declared--;
                        num_variables++;
                    }

                }

                else{
                    check_heap = is_heap_RO(line);
                    temp = strtok(NULL, " "); //gets the variable name
                    temp = remove_equal_colon_bracket(temp); //gets rid of the "=" or ";" from the name of the variable.
                    var_name = strdup(temp);
                    var_name = remove_whitespace(var_name);

                    temp_var_head = insert_var(temp_var_head, remove_stars(var_name), fun_name, datatype, size, get_sizeof(line, &size), -1, -1, check_heap);
                    num_variables++;
                }
            }

            else{
                char *same_name = remove_whitespace(remove_equal_colon_bracket(line));
                check_heap = is_heap_RO(line);
                add_updated_var(same_name, temp_var_head, global_head, fun_name, check_heap, line);
            }
        }

        else{
            char *same_name = remove_whitespace(remove_equal_colon_bracket(line));
            check_heap = is_heap_RO(line);
            add_updated_var(same_name, temp_var_head, global_head, fun_name, check_heap, line);
        }
        

        strcpy(prev_line, line);
    }

    VARNode *temp = temp_var_head;

    while (temp != NULL){
        temp->num_lines_function = total_lines;
        temp->num_var_function = num_variables;
        temp = temp->next;
    }

    temp = temp_var_head;

    while (temp != NULL){
        var_head = insert_var(var_head, temp->name, temp->scope, temp->type, 
            temp->size, temp ->data_size, temp->num_lines_function, temp->num_var_function, temp->heap_stack);

        temp = temp -> next;
    }
    
    return var_head;
}

//returns 1 if name already belongs to the linked list
int if_belongs(FUNNode *head, char *name){
    FUNNode *temp = head;

    while (temp != NULL){
        if (strcmp(temp->name, name) == 0){
            return 1;
        }

        temp = temp -> next;
    }

    return 0;
}

//option 1, add * at front, option 2 add at back
char *add_star(char *name, int option){
    char *datasize = malloc(sizeof(char) * MAX_LENGTH);
    int index = 0;
    int i = 0;

    if (option == 1){
        datasize[index] = '*';
        index += 1;
        while(name[i] != '\0'){
            datasize[index] = name[i];
            index++;
            i++;
        }

        datasize[index] = '\0';
    }

    else{
        while(name[i] != '\0'){
            datasize[index] = name[i];
            index++;
            i++;
        }

        datasize[index] = '*';
        datasize[index+1] = '\0';
    }
    

    return datasize;
}
//prints the line to stdout, choosing whether to have size as int or string
void print_correct_line(VARNode *head){

    if (head->heap_stack == 1 || head->heap_stack == 3){
        head->name = strdup(add_star(head->name, 1));
    }

    if (head->size == -1 || head->heap_stack == 1){
        printf("\t%s   %s   %s   %s\n", head->name, head->scope, head->type, head->data_size);
    }

    else{
        printf("\t%s   %s   %s   %d\n", head->name, head->scope, head->type, head->size);
    }
}

//the controller function of entire program
//prints out all the required data, based on where it should be printed
void analyze_source(FILE *fp, char *path){
    VARNode *global_head = NULL;
    global_head = find_global_var(fp, path, global_head);

    fp = fopen(path, "r");
    char line[MAX_LENGTH];
    char temp_line[MAX_LENGTH];

    VARNode *head = NULL;

    while (fgets(line, MAX_LENGTH, fp)){
        strcpy(line, cleanline(line));
        if (strstr(line, "{")){ 
            /*
                '{' signifies starting of a new block. It can either be

                (1) a function
                (2) a loop- assume only either for loop or while loop
                (3) if/else statement
            */

           if (check_function_datatype(temp_line) == 1){
                int variables = num_variables_header(temp_line);

                head = analyze_function(fp, temp_line, head, global_head);
           } 

        }        
        strcpy(temp_line, line);
    }
    fclose(fp);

    printf("### ROData ### \t\t scope type size\n");

    VARNode *ro_temp = head;

    while (ro_temp != NULL){
        if (ro_temp->heap_stack == 3){
            print_correct_line(ro_temp);
        }

        ro_temp = ro_temp -> next;
    }

    printf("\n### static data ###\n");
    
    VARNode *global_temp = global_head;
    while (global_temp != NULL){
        print_correct_line(global_temp);
        global_temp = global_temp -> next;
    }

    printf("\n### heap ###\n");
    
    global_temp = global_head;
    while (global_temp != NULL){
        if (global_temp->heap_stack == 1){
            print_correct_line(global_temp);
        }

        global_temp = global_temp -> next;
    }

    VARNode *heap_temp = head;
    while (heap_temp != NULL){
        if (heap_temp->heap_stack == 1){
            heap_temp->type = remove_stars(heap_temp->type);
            print_correct_line(heap_temp);
            heap_temp->name = remove_stars(heap_temp->name);

            heap_temp->heap_stack = 2;
            heap_temp->type = add_star(heap_temp->type, 2);

            heap_temp->data_size = get_sizeof(heap_temp->type, &(heap_temp->size));
        }

        heap_temp = heap_temp -> next;
    }

    printf("\n####################\n");
    printf("### unused space ###\n");
    printf("####################\n\n");

    printf("## stack ###\n");


    VARNode *stack_temp = head;
    int num_stack_var = 0;
    while (stack_temp != NULL){
        if (stack_temp->heap_stack == 2){
            print_correct_line(stack_temp);
        }

        num_stack_var++;
        stack_temp = stack_temp -> next;
    }

    printf("\n**** STATS ****\n");
    int total_lines = line_counter(fp, path, "");
    printf("\t- Total number of lines in the file: %d\n", total_lines);

    FUNNode *fun_head = NULL;
    VARNode *total_fun_temp = head;

    //creates the linked list for the functions
    while (total_fun_temp != NULL){
        if (if_belongs(fun_head, total_fun_temp->scope) == 0){
            fun_head = insert_fun(fun_head, total_fun_temp->scope, total_fun_temp->num_lines_function, total_fun_temp->num_var_function);
        }

        total_fun_temp = total_fun_temp -> next;
    }

    //get the total number of functions 
    int total_functions = 0;
    FUNNode *temp_total_fun = fun_head;
    while (temp_total_fun != NULL){
        total_functions++;
        temp_total_fun = temp_total_fun -> next;
    }
    
    printf("\t- Total number of functions: %d\n", total_functions);
    printf("\t  ");

    //print all the functions 
    temp_total_fun = fun_head;
    while (temp_total_fun != NULL){
        if (temp_total_fun -> next != NULL){
            printf("%s, ", temp_total_fun->name);
        }

        else{
            printf("%s\n", temp_total_fun->name);
        }
        temp_total_fun = temp_total_fun -> next;
    }

    //print total lines per function 
    printf("\t- Total number of lines per functions:\n");
    temp_total_fun = fun_head;
    while (temp_total_fun != NULL){
        printf("\t  %s: %d\n", temp_total_fun -> name, temp_total_fun -> total_lines);
        temp_total_fun = temp_total_fun -> next;
    }

    //print total variables per function 
    printf("\t- Total number of variables per functions:\n");
    temp_total_fun = fun_head;
    while (temp_total_fun != NULL){
        printf("\t  %s: %d\n", temp_total_fun -> name, temp_total_fun -> total_variables);
        temp_total_fun = temp_total_fun -> next;
    }
    
    printf("//////////////////////////////\n");    
}

//returns 1 if valid file, 0 if not
//https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
int valid_file(char *path)
{
    FILE *fp;
    if ((fp = fopen(path, "r")))
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

//Determines if command line arguments are valid
int main(int argc , char **argv){ 
    FILE *source; 

    //if no file name is given to be analyzed
    if (argc < 2){ 
        fprintf(stderr, "Not enough arguments\n");
    }

    else if (argc == 2){
        if (source != NULL){
            char path[MAX_LENGTH];
            strcpy(path, "./"); //to access the file in the current directory
            strcat(path, argv[1]);

            if (valid_file(path) == 1){
                printf(">>> Memory Model Layout <<<\n");
                printf("\n***  exec // text ***\n");
                printf("\t%s\n\n", argv[1]);

                analyze_source(source, path);
            }

            else{
                fprintf(stderr, "File is invalid\n");
            }
        }

        else{
            fprintf(stderr, "File is NULL\n");
        }        
    }

    //if too many arguments are given 
    else{
        fprintf(stderr, "Too many arguments\n");
    }
}