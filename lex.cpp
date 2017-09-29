#include <iostream>
#include <fstream>
#include <string>
#include < list > 
#include < vector > 

struct evl_token {
    enum token_type {NAME, NUMBER, SINGLE};
    token_type type;
    std::string str;
    int line_no;
}; // struct evl_token
typedef std::list<evl_token> evl_tokens;

struct evl_statement {
    enum statement_type {MODULE, WIRE, COMPONENT, ENDMODULE};
    statement_type type;
    evl_tokens tokens;
}; // struct evl_statement
typedef std::list<evl_statement> evl_statements;

struct evl_wire
{
    std::string name;
    int width;
}; // struct evl_wire
typedef std::list<evl_wire> evl_wires;

struct evl_pin
{
    std::string name;
    int msb_p;
    int lsb_p;
}; // struct pin
typedef std::list<evl_pin> evl_pins;

struct evl_component
{
    std::string name;
    int num_p;
    std::list<evl_pin> pins;
}; // struct evl_component
typedef std::list<evl_component> evl_components;

void display_wire(evl_wires wires, int num_w,std::string output_file_name);
void display_component(evl_components components, int num_c,std::string output_file_name);
evl_statements t_s(evl_tokens tokens);

std::list<evl_token>::iterator k;
std::list<evl_statement>::iterator h;

int main(int argc, char *argv[])
{
    evl_tokens tokens;
    evl_token tmp_t;

    evl_statements statements;
    evl_statement tmp_s;

    evl_wires wires;
    evl_wire tmp_w;

    evl_components components;
    evl_component tmp_c;

    evl_pins pins;
    evl_pin tmp_p;

    int msb=0, lsb=0;
    int num_w=0;
    int num_c=0;
    int width=1;

    if (argc < 2)
    {
        std::cerr << "You should provide a file name." << std::endl;
        return -1;
    }

    std::ifstream input_file(argv[1]);
    if (!input_file)
    {
        std::cerr << "I can't read " << argv[1] << "." << std::endl;
        return -1;
    }

    std::string output_file_name = std::string(argv[1])+".syntax";
    std::ofstream output_file(output_file_name);
    if (!output_file)
    {
        std::cerr << "I can't write " << argv[1] << ".syntax ." << std::endl;
        return -1;
    }

    std::string line;
    for (int line_no = 1; std::getline(input_file, line); ++line_no)
    {
        for (size_t i = 0; i < line.size();)
        {
            // comments
            if (line[i] == '/')
            {
                ++i;
                if ((i == line.size()) || (line[i] != '/'))
                {
                    std::cerr << "LINE " << line_no
                        << ": a single / is not allowed" << std::endl;
                    return -1;
                }
                break; // skip the rest of the line by exiting the loop
            }
            // spaces
            if ((line[i] == ' ') || (line[i] == '\t')
                || (line[i] == '\r') || (line[i] == '\n'))
            {
                ++i; // skip this space character
                continue; // skip the rest of the iteration
            }

            // SINGLE
            if ((line[i] == '(') || (line[i] == ')')
                || (line[i] == '[') || (line[i] == ']')
                || (line[i] == ':') || (line[i] == ';')
                || (line[i] == ','))
            {
                //-----------------------------------------------------------------------
                tmp_t.type=evl_token::token_type::SINGLE;
                tmp_t.str=line[i];

                tokens.push_back(tmp_t);
                ++i;
                continue; // skip the rest of the iteration
            }
            // NAME
            if (((line[i] >= 'a') && (line[i] <= 'z'))       // a to z
                || ((line[i] >= 'A') && (line[i] <= 'Z'))    // A to Z
                || (line[i] == '_'))
            {
                size_t name_begin = i;
                for (++i; i < line.size(); ++i)
                {
                    if (!(((line[i] >= 'a') && (line[i] <= 'z'))
                        || ((line[i] >= 'A') && (line[i] <= 'Z'))
                        || ((line[i] >= '0') && (line[i] <= '9'))
                        || (line[i] == '_') || (line[i] == '$')))
                    {
                        break; // [name_begin, i) is the range for the token
                    }
                }
                //----------------------------------------------------------------------
                tmp_t.type=evl_token::token_type::NAME;
                tmp_t.str=line.substr(name_begin, i-name_begin);

                tokens.push_back(tmp_t);

                continue;
            }
            //NUMBER
            if((line[i]>='0')&&(line[i]<='9'))
            {
                size_t name_begin = i;
                for(++i; i< line.size(); i++)
                {
                    if(!((line[i] >= '0')&&(line[i] <= '9')))
                        break;
                }
                //----------------------------------------------------------------------
                tmp_t.type=evl_token::token_type::NUMBER;
                tmp_t.str=line.substr(name_begin, i-name_begin);

                tokens.push_back(tmp_t);

                continue;
            }
            else
            {
                std::cerr << "LINE " << line_no
                    << ": invalid character" << std::endl;
                return -1;
            }
        }
    }

    //tokens->statements
    statements=t_s(tokens);
    
    //statements->wires and components
    std::cout<<"begin()--- end():"<<std::endl;   
    for (h= statements.begin(); h!= statements .end(); ++h)   
    {
        tmp_s= *h;
        if(tmp_s.type==evl_statement::statement_type::MODULE)
        {

            for (; !tmp_s.tokens.empty();) {
                tmp_t=tmp_s.tokens.front();
                tmp_s.tokens.pop_front();
                std::cout<<tmp_t.str<<"-";
                if (tmp_s.tokens.front().str == ";")
                    break; // exit if the ending ";" is found
            }
        }
        if(tmp_s.type==evl_statement::statement_type::WIRE)
        {

            for (; !tmp_s.tokens.empty();) 
            {
                tmp_t=tmp_s.tokens.front();

                if (tmp_s.tokens.front().str == ","|| tmp_s.tokens.front().str == "]")
                    tmp_s.tokens.pop_front();

                if (tmp_s.tokens.front().str == ";")
                    break; // exit if the ending ";" is found

                if (tmp_s.tokens.front().str == "[")
                {
                    tmp_s.tokens.pop_front();    //from [ to msb
                    tmp_t=tmp_s.tokens.front();  
                    msb=atoi(tmp_t.str.c_str()); //msb

                    tmp_s.tokens.pop_front();    //from msb to :

                    tmp_s.tokens.pop_front();    //from : to lsb
                    tmp_t=tmp_s.tokens.front();  
                    lsb=atoi(tmp_t.str.c_str()); //msb

                    width=msb+1-lsb; 
                    tmp_s.tokens.pop_front(); //from lsb to name
                }
                else
                {
                    tmp_t=tmp_s.tokens.front();
                    tmp_w.width=width; 
                    tmp_w.name=tmp_t.str;
                    tmp_s.tokens.pop_front(); //from name to , or ;
                    wires.push_back(tmp_w);
                    num_w++;
                }


            }
        }

        if(tmp_s.type==evl_statement::statement_type::COMPONENT)
        {
            num_c++;
            tmp_t=tmp_s.tokens.front();
            tmp_c.name=tmp_t.str;
            tmp_c.num_p=0;
            tmp_s.tokens.pop_front();  //from c_name to (
            tmp_c.pins.clear();
            for (; !tmp_s.tokens.empty();) 
            {
                tmp_t=tmp_s.tokens.front();    
                if (tmp_s.tokens.front().str == ";")
                {
                    components.push_back(tmp_c);
                    break; // exit if the ending ";" is found
                }
                else if (tmp_s.tokens.front().str == "(" || tmp_s.tokens.front().str == ","
                    || tmp_s.tokens.front().str == ")" )
                {
                    tmp_s.tokens.pop_front();
                    continue;
                } 
                else
                {
                    tmp_c.num_p++;
                    tmp_p.name=tmp_t.str;
                   // tmp_c.pins.push_back(tmp_t.str);
                    tmp_s.tokens.pop_front();   //from pin to , or )or[
                    if(tmp_s.tokens.front().str == "[")
                    {
                        tmp_s.tokens.pop_front();   //from [ to msb
                        tmp_t=tmp_s.tokens.front();  
                        tmp_p.msb_p=atoi(tmp_t.str.c_str()); //msb

                        tmp_s.tokens.pop_front();   //from msb tp : or ]
                        if(tmp_s.tokens.front().str == ":")
                        {
                            tmp_s.tokens.pop_front();   //from :to lsb
                            tmp_t=tmp_s.tokens.front();  
                            tmp_p.lsb_p=atoi(tmp_t.str.c_str()); //lsb
                            tmp_s.tokens.pop_front(); //from lsb to]
                            tmp_s.tokens.pop_front();  ////from ] to,
                            tmp_s.tokens.pop_front();  ////from , to pin
                        }
                        if(tmp_s.tokens.front().str == "]")
                        {
                            tmp_p.lsb_p=atoi(tmp_t.str.c_str()); //lsb
                            tmp_s.tokens.pop_front();  ////from ] to,
                            tmp_s.tokens.pop_front();  ////from , to pin
                        }
                    }
                    else
                    {
                        tmp_p.msb_p=-1; //msb without []
                        tmp_p.lsb_p=-1; //lsb without []
                    }
                     tmp_c.pins.push_back(tmp_p);
                }
            }
        }
    }
    output_file <<"module top"<< std::endl; 

    //for (; !components.empty();) {
    //    std::cout <<components.front().name<< std::endl; 
    //    for (; !components.front().pins.empty();) {
    //        std::cout <<components.front().pins.front()<< std::endl; 
    //        components.front().pins.pop_front();
    //    }
    //    components.pop_front();
    //}

    //display_wire(wires,num_w,output_file_name);
     output_file<<"wires "<<num_w<<std::endl;
    for(;num_w>0;num_w--)
    {
        output_file<<"    wire "<<wires.front().name<<" "
            <<wires.front().width<<std::endl;
        wires.pop_front();
    }
    //display_component(components, num_c,output_file_name);
    output_file<<"components "<<num_c<<std::endl;
    for(;num_c>0;num_c--)
    {
        output_file<<"    component "<<components.front().name<<" "
            <<components.front().num_p<<std::endl;
        for(;components.front().num_p>0;components.front().num_p--)
        {
            output_file<<"        pin "<<components.front().pins.front().name;
            if(components.front().pins.front().msb_p==-1)
            {
                output_file<<std::endl;
                components.front().pins.pop_front();
                continue;
            }
            else
            {
                output_file<<"["<<components.front().pins.front().msb_p<<"]["
                    <<components.front().pins.front().lsb_p<<"]"<<std::endl;
                components.front().pins.pop_front();
            }
        }
        components.pop_front();
    }
}
void display_wire(evl_wires wires, int num_w,std::string output_file_name)
{
    std::ofstream output_file(output_file_name);
    output_file<<"wires "<<num_w<<std::endl;
    for(;num_w>0;num_w--)
    {
        output_file<<"    wire "<<wires.front().name<<" "
            <<wires.front().width<<std::endl;
        wires.pop_front();
    }
}

void display_component(evl_components components, int num_c,std::string output_file_name)
{
    std::ofstream output_file(output_file_name);
     for(;num_c>0;num_c--)
    {
        output_file<<"    component "<<components.front().name<<" "
            <<components.front().num_p<<std::endl;
        for(;components.front().num_p>0;components.front().num_p--)
        {
            output_file<<"        pin "<<components.front().pins.front().name;
            if(components.front().pins.front().msb_p==-1)
            {
                output_file<<std::endl;
                components.front().pins.pop_front();
                continue;
            }
            else
            {
                output_file<<"["<<components.front().pins.front().msb_p<<"]["
                    <<components.front().pins.front().lsb_p<<"]"<<std::endl;
                components.front().pins.pop_front();
            }
        }
        components.pop_front();
    }
}

evl_statements t_s(evl_tokens tokens)
{
    evl_token tmp_t;

    evl_statements statements;
    evl_statement tmp_s;
    int flag=0;

    std::cout<<"tokens->statements"<<std::endl;   
    for (k= tokens.begin(); k != tokens.end(); ++k)   
    {
        tmp_t= *k;
        if(tmp_t.str==";")
        {
            tmp_s.tokens.push_back(tmp_t);  //push ;
            statements.push_back(tmp_s);
            tmp_s.tokens.clear();
            flag=0;
        }
        else
        {
            if(flag==0)
            {
                std::cout<<"new statement"<<std::endl;
                if(tmp_t.str=="module")
                    tmp_s.type=evl_statement::statement_type::MODULE;
                else if(tmp_t.str=="endmodule")
                    tmp_s.type=evl_statement::statement_type::ENDMODULE;
                else if(tmp_t.str=="wire")
                    tmp_s.type=evl_statement::statement_type::WIRE;
                else
                {
                    tmp_s.type=evl_statement::statement_type::COMPONENT;
                    tmp_s.tokens.push_back(tmp_t);
                }
                flag=1;
                continue;
            }
            tmp_s.tokens.push_back(tmp_t);
        }
    }
    return statements;
}