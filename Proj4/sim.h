#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <assert.h>
#include <math.h>
#include <sstream>

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

struct evl_module
{
    std::string name;
    evl_wires m_wires;
    evl_components m_components;
    int n_w;
    int n_c;
}; // struct evl_module
typedef std::list<evl_module> evl_modules;

typedef std::map<std::string, int> evl_wires_table;


class netlist;
class gate;
class net;
class pin;
class net {
    std::string name_; // e.g. "a", "s[0]"
    char signal_; // e.g. '0' or '1'
    std::list<pin *> connections_; // relationship "connect"
public:
    net(const std::string net_name){name_=net_name;};
    void append_pin(pin *p);

    char get_signal();
    void set_signal(char s);

    void display_nets(std::ofstream &output_file);
}; // class net

class pin {
    char dir_; // e.g. 'I' or 'O'
    gate *gate_; // relationship "contain"
    size_t index_; // attribute of "contain"
    net *net_; // relationship "connect"
    std::string name_;
    int lsb;
    int width;
public:
    bool create(gate *g, size_t index, const evl_pin &p, const std::map<std::string, net *> &nets_table);

    char compute_signal();

    void display_connections(std::ofstream &output_file);
    void set_as_input();
    void set_as_output();
    int get_width();
    int get_lsb();
    char get_dir();
    std::string get_name();
}; // class pin

class gate {
    std::string name_;
    std::string type_; // e.g. "and", "or"
    std::list<pin *> pins_; // relationship "contain"
    std::list<std::string> input_;
    char next_state_;
    char state_;
    int trans;
    
public:
    bool create(const evl_component &c, const std::map<std::string, net *> &nets_table, const evl_wires_table &wires_table);
    bool create_pin(const evl_pin &ep, size_t index, const std::map<std::string, net *> &nets_table, const evl_wires_table &wires_table);
    std::string get_name();

    void compute_next_state_or_output(std::map<std::string, net *> nets_table);
    char compute_signal(int pin_index);
    bool validate_structural_semantics();
    void change_state();
}; // class gate

class netlist {
    std::list<gate *> gates_;
    std::list<net *> nets_;
    std::map<std::string, net *> nets_table;
    //iot +
public:
    bool create(const evl_wires &wires,   const evl_components &comps,  const evl_wires_table &wires_table);
    bool create_nets(const evl_wires &wires);
    bool create_gates(const evl_components &comps,  const evl_wires_table &wires_table );
    void create_net(std::string net_name);
    bool create_gate(const evl_component &c, const evl_wires_table &wires_table);

    void compute_next_state_and_output();

    void display_netlist(evl_module tmp_m,std::ofstream &output_file);
}; // class netlist

evl_statements t_s(evl_tokens tokens);
int display_syn(evl_module tmp_m,std::ofstream &output_file);
void display_comp(evl_module tmp_m,std::ofstream &output_file,const evl_wires_table &wires_table);
bool make_wires_table(const evl_wires &wires,evl_wires_table &wires_table);
std::string make_net_name(std::string wire_name, int i) ;
void  undate_pin(evl_module &tmp_m,const evl_wires_table &wires_table);
void get_input(std::list<std::string> &line_,std::string line_s);

std::list<evl_token>::iterator k;
std::list<evl_statement>::iterator h;
std::string filename_;