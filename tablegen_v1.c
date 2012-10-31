
/*
 *  tt2ht3.c

 *  purp: convert plain text tabular data into html markup, process plain-text, 
          process attributes, process dyanmic delimiters for table gen.
 *  args: none
 *  rets: nothing, 
 *  inpt: reads from stdin, expects text data table, no-process html, or attributes 
 *  outp: writes to stdout, html marked up text
 */

#include <stdio.h> 
#include <string.h> 

/* ---------------------  Define Constants & Variables  --------------------- */

// PROCESS STATE
    int process_state = 0;
        //[0] init, [1] print text, [2] store attributes, [3] generate table

// INDENT
    #define INDENT "    "
    #define INDENT_BASE 0

// control indentation level
    int indent_state;
    char indent[40];

// indexing rows and fields
    int table_ind = 0;
    int row_ind = 0;
    int field_ind = 0;

// TAGS
    char tag_type[10]; 
    char element_class_type[20];
    char post_tag[2];

// ATTRIBUTES
    #define MAX_ATTRIBUTES 20
    #define MAX_ATTRIBUTES_LEN 30
    char attributes[80];
    int attribute_index;
    char var[MAX_ATTRIBUTES][MAX_ATTRIBUTES_LEN];
    int attribute_status = 0;
    int attribute_state = 0;

    // turns on auto classes: ex. [ class="field_1" ] joining element + index
    // see function set_tag_attributes()
    int enable_auto_class = 1; // extended functionality


// LINES FOR PARSING
    #define MAX_LINE_LEN 500
    char line[MAX_LINE_LEN];
    char trimmed_str[MAX_LINE_LEN];

// DELIMITER
    char *delsym;
    char delim[2];

// FUNCTIONS
    char *trim_newline(char trim_str[MAX_LINE_LEN] );
    void set_indent();
    void scan_delim();
    void set_delim();
    void scan_noprocess();
    void scan_attributes();
    void set_tag_attributes( char _element_class_type[], int _tag_ind);
    void set_tag_properties(           \
        char _tag_type[],              \
        int _tag_ind,                  \
        char _element_class_type[],    \
        int _indent_state,             \
        char _post_tag[] );

    void markup( int tag_state, char tag_type_state[], int tag_ind);
    void print_tag(int tag_state);

/*  
main() Model:

Reads input by each line, 
    opens row
        iterates over fields, 
        prints: <html open markup> [field] <html close markup> 
    closes row
return 0
*/
int main () 
{ 
    /* ----------------------------  Initialize  ---------------------------- */

    // stores tag type for printing markup ['tr', 'td']
    char tag_type_state[10];
        strcpy(post_tag, "");

    // delimiter for strtok field parsing
    strcpy(delim, " ");

    // parsed field for printing
    char * field; 

    /* -----------------------------  Process  ------------------------------ */
    //strcpy(tag_type_state, "table"); // set tag_type to table
    //markup(1, "table", table_ind); // open table
    process_state = 3 ;
    while(  fgets( line, MAX_LINE_LEN , stdin )  )
    {
        
        scan_delim();
        scan_noprocess();
        scan_attributes();

        if ( strcmp(line, "\n") == 0 )
        {
            continue;
        }

        switch (process_state)
        {
        case 1:
            printf("%s", line);
            break;

        case 3:
            if (strcmp(line,"") == 0 ){continue;}

            // trim last char newline to allow strtok and markup proper format
            strcpy(line, trim_newline(line));
            row_ind++; // next row

            //strcpy(tag_type_state, "row"); // set tag_type to row
            markup(1, "row", row_ind); // open row

            // CHANGE TO DELIM

            field = strtok (line, delim); 
            while (field != NULL) 
            { 
                //printf("\n ~ delimiter = [%s] ~ \n", delim );

                field_ind++; // next field

                //strcpy(tag_type_state, "field"); // set tag_type to cell
                markup(1, "field", field_ind); // open cell
                printf ("%s", field);          // print cell value
                markup(2, "field", field_ind); // close cell

                field = strtok (NULL, delim); 

            } 
            field_ind = 0; // reset field index

            //strcpy(tag_type_state, "row"); // set tag_type to row
            markup(2, "row", row_ind);  // close row
            break;

        default:
            break;
        }
    } 
    //strcpy(tag_type_state, "table"); // set tag_type to table
    //markup(2, "table", table_ind); // close table

    return 0; // - done. -
}

/* -------------------------  Trim Newline Ending  -------------------------- */
/*
 *  purp: trim newline \n character off end for proper tag indent formatting
 *  args: trim_str: tring to be trimmed and returned
 *  rets: trimmed_str; trimmed string, 
*/
char *trim_newline(char trim_str[MAX_LINE_LEN]){
    strcpy(trimmed_str, trim_str);

    if (trimmed_str[strlen(trimmed_str) - 1] == '\n'){
        trimmed_str[strlen(trimmed_str) - 1] = '\0';
    }
    return trimmed_str;
}

/* ------------------------------  Set Indent  ------------------------------ */
/*
 *  purp: dynamically set indent level for markup printing
 *  args: none
 *  rets: none
 *  note: sets top-level 'indent' var 
*/
void set_indent()
{
    int indent_index;

    strcpy(indent, "");
    if (indent_state > 0){
        for( indent_index = 0; indent_index < indent_state; indent_index++){
            strcat(indent, INDENT);
        }
    }
    else{
        strcpy(indent, ""); // no indent
    }
}

/* ------------------------  Check Delimiter Input  ------------------------- */
/*
 *  purp: checks line for delimiter specification, triggers set_delim()
 *  args: none, analyzes 'line'
 *  rets: none 
 *  uses: set_delim()
*/
void scan_delim()
{ 
    // if we hit a [ <delim value=" ] * [ />\n ]
    if ( (strstr(line, "<delim value=") != NULL) && (strstr(line, "/>\n") != NULL) )
    {
        set_delim();
    }
}

/* -------------------------  Set Delimiter Symbol  ------------------------- */
/*
 *  purp: sets the delimiter symbol to specification
 *  args: none, analyzes 'line'
 *  rets: none 
*/
void set_delim()
{ 
    // extract delimiter
    char delim_tag[30];
    strcpy(delim_tag, line);
    strcpy(delim , strtok(delim_tag, "="));
    strcpy(delim , strtok (NULL, "/"));
    strcpy(line, ""); // clear line for next step
}

/* ------------------------  Check No-Process Input  ------------------------ */
/*
 *  purp: checks line for <noprocess> tags and switches process state
 *  args: none, analyzes 'line'
 *  rets: trimmed_str; trimmed string, 
*/
void scan_noprocess()
{   
    // if we hit a <attributes>
    if (strcmp(line, "<noprocess>\n") == 0) 
    {
        process_state = 1;
        strcpy(line, "");
    }

    // if we hit a </attributes>
    if (strcmp(line, "</noprocess>\n") == 0) 
    {
        process_state = 3;
        strcpy(line, ""); 
    }
}

/* ------------------------  Check Attribute Input  ------------------------- */
/*
 *  purp: checks line for <attribute> tags, stores attributes, switches process
 *  args: none, analyzes 'line'
 *  rets: none
 *  uses: trim_newline()
*/
 void scan_attributes()
{   
    // if we hit a <attributes>
    if (strcmp(line, "<attributes>\n") == 0) 
    {
        process_state = 0;
        attribute_state = 1;
        attribute_status = 1; // attributes have been activated
        strcpy(line, "");
        return;
    }

    // if we hit a </attributes>
    if (strcmp(line, "</attributes>\n") == 0) 
    {
        attribute_state = 0;
        process_state = 3;
        strcpy(line, "");
        return;
    }

    switch (attribute_state)
    {
        case 0:
            // if hit <attribute> open tag, reset index
            attribute_index = 0;
            break;
        case 1:
            // else iterate to next attribute
            
            strcpy(var[attribute_index], trim_newline(line));
            attribute_index++;
            strcpy(line, "");
            break;
    }
}

/* -------------------------  Set Tag Attributes  --------------------------- */
/*
 *  purp: sets the tag attributes for printing markup
 *  args: char _element_class_type[]: type of element, used in logic, classes
 *                      int _tag_ind: element index, access attributes, classes
 *  rets: none
 *  note: supports auto-class markup, extended feature.
*/
void set_tag_attributes( char _element_class_type[], int _tag_ind)
{
    int attr_len;
    if ( attribute_status == 1){
        if (( strcmp(_element_class_type, "field") == 0 )         \
            &&                                                    \
            ( (attr_len = strlen(var[_tag_ind-1]))  > 2 )         \
            ) 
        {
                sprintf(attributes, " %s", var[_tag_ind-1]);
                // IF strstr(attributes 'class="*"');
                        //insert (element_class_type"_"tag_ind)
                        //into class="*" 
        }
        else 
        {
            if (enable_auto_class  == 0)
            {
                strcpy( attributes, "");
            }
            else { // print default classes for CSS access
                sprintf(attributes, " class=\"%s_%d\"",              \
                                     _element_class_type,            \
                                     _tag_ind);       
            }
        }
    }
    else if ( attribute_status == 0 ){

        // turn on 'enable_auto_class' at top-level initilization
        if (enable_auto_class == 1 ){
            // print default classes for CSS access
            sprintf(attributes, " class=\"%s_%d\"",              \
                                 _element_class_type,            \
                                 _tag_ind);       
        }
        // default: display no classes if no specified <attributes> 
        else{
            strcpy(attributes, "");
            
        }
    }
}

/* --------------------------  Set Tag Propeties  --------------------------- */
/*
 *  purp: set all tag properties for printing markup
 *  args:     char _tag_type[]: element tag
 *                 int _tag_ind: element index, access attributes, classes
 *     char _element_class_type: type of element, used in logic, classes
 *            int _indent_state: level of indent
 *               char _post_tag: any optional markup after tag
 *  rets: none
 *  uses: 
 *  note: stores properties in top-level vars for proccessing by print_tag().
*/
void set_tag_properties(                            \
    char _tag_type[],                               \
    int _tag_ind,                                   \
    char _element_class_type[],                     \
    int _indent_state,                              \
    char _post_tag[] )
{
    indent_state = _indent_state;
    set_indent();
    strcpy(tag_type,_tag_type);
    strcpy(element_class_type, _element_class_type); 
    set_tag_attributes(_element_class_type, _tag_ind);
    strcpy(post_tag, _post_tag);
}

/* ------------------------------  Markup  ------------------------------- */
/*
 *   purp: convert plain text tabular data into html markup
 *   args:        tag_state: 1 = open, 2 = close.
 *           tag_type_state: 'tr', 'td', etc.
 *                  tag_ind: handles index for rows, fields, etc
 *   rets: nothing, 
 *   inpt: triggered to print open/close tags during line/field parsing
 *   outp: writes to stdout, html tags w markup attributes

 *   First we check type of tag and set the tag properties,
 *   Then we check if tag is openning or closing to print proper markup.
*/

void markup(int tag_state, char tag_type_state[], int tag_ind) 
{  
    if (strcmp(tag_type_state, "table") == 0) 
    {
        set_tag_properties("table", tag_ind, "table_gen", INDENT_BASE, "\n" );
    } 
    if (strcmp(tag_type_state, "row") == 0) 
    {
        set_tag_properties("tr", tag_ind, tag_type_state, INDENT_BASE+1, "\n");
    } 
    else if (strcmp(tag_type_state, "field") == 0) 
    {
        if (tag_state == 1){ // open
            indent_state = INDENT_BASE + 2; // double indent
        }
        if (tag_state == 2){ // close
            indent_state = 0; // no indent
        }
        set_tag_properties("td", tag_ind, tag_type_state, indent_state, "");
    }

   print_tag(tag_state);
}

/* ---------------------------  Print Tag Markup  --------------------------- */
/*
 *  purp: print tag markup to output
 *  args: int tag_state: control open / close tagging
 *  rets: none
*/
void print_tag(int tag_state)
{
    switch(tag_state){
        case 1: // openning
        // print the openning tag and properties   
        printf ("%s<%s%s>%s", indent, tag_type, attributes, post_tag);
        break;

        case 2: // closing
        // print the closing tag, handle line termination;
        printf("%s</%s>\n",indent, tag_type);
        break;
    }

}

