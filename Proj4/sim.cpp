#include "sim.h"

int main(int argc, char *argv[])
{

    evl_tokens tokens;
    evl_token tmp_t;

    evl_statements statements;
    evl_statement tmp_s;

    evl_modules modules;
    evl_module tmp_m;

    evl_wires wires;
    evl_wire tmp_w;

    evl_components components;
    evl_component tmp_c;

    evl_pins pins;
    evl_pin tmp_p;

    evl_wires_table wires_table;

    int msb=0, lsb=0;
    int num_w=0;
    int num_c=0;
    int width=1;

    argc=2;
    argv[1]="test.evl";

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
    filename_=argv[1];
    //std::string output_file_name = std::string(argv[1])+".netlist";
    //std::ofstream output_file(output_file_name);
    //if (!output_file)
    //{
    //    std::cerr << "I can't write " << argv[1] << ".netlist." << std::endl;
    //    return -1;
    //}

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
    for (h= statements.begin(); h!= statements .end(); ++h)   
    {
        tmp_s= *h;
        if(tmp_s.type==evl_statement::statement_type::MODULE)
        {

            for (; !tmp_s.tokens.empty();) {
                tmp_t=tmp_s.tokens.front();
                tmp_m.name=tmp_t.str;
                tmp_s.tokens.pop_front();
                if (tmp_s.tokens.front().str == ";")
                {
                    //modules.push_back(tmp_m);
                    break; // exit if the ending ";" is found
                }
            }
        }
        if(tmp_s.type==evl_statement::statement_type::WIRE)
        {
            width=1;
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
                    lsb=atoi(tmp_t.str.c_str()); //lsb

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
            //tmp_c.name=tmp_t.str;
            //tmp_s.tokens.pop_front();  //from c_name to (
            tmp_c.name=tmp_t.str;
            tmp_s.tokens.pop_front(); 
            while(tmp_s.tokens.front().str != "(")
            {
                tmp_c.name=tmp_c.name+" "+tmp_s.tokens.front().str;
                tmp_s.tokens.pop_front(); 
            }
            tmp_c.num_p=0;
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

    tmp_m.n_c=num_c;
    tmp_m.n_w=num_w;
    tmp_m.m_wires=wires;
    tmp_m.m_components=components;
    modules.push_back(tmp_m);

    make_wires_table(wires,wires_table);

    for (evl_wires_table::const_iterator it = wires_table.begin();it != wires_table.end(); ++it) 
    {
        std::cout << "wire " << it->first<< " " << it->second << std::endl;
    }
    undate_pin(tmp_m,wires_table);
    //display_syn(tmp_m, output_file);
    netlist nl;
    if (!nl.create(tmp_m.m_wires, tmp_m.m_components, wires_table))
        return -1;

    //nl.display_netlist(tmp_m, output_file);
  //  display_comp(tmp_m,output_file,wires_table);
    nl.compute_next_state_and_output();
}

evl_statements t_s(evl_tokens tokens)
{
    evl_token tmp_t;

    evl_statements statements;
    evl_statement tmp_s;
    int flag=0;

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

int display_syn(evl_module tmp_m,std::ofstream &output_file)
{
    //display module
    output_file<<"module "<<tmp_m.name<<std::endl;
    //display_wire(wires,num_w,output_file_name);
    if(tmp_m.n_w!=0)
    {
        output_file<<"wires "<<tmp_m.n_w<<std::endl;
        for(;tmp_m.n_w>0;tmp_m.n_w--)
        {
            output_file<<"    wire "<<tmp_m.m_wires.front().name<<" "
                <<tmp_m.m_wires.front().width<<std::endl;
            tmp_m.m_wires.pop_front();
        }
    }
    //display_component(components, num_c,output_file_name);
    output_file<<"components "<<tmp_m.n_c<<std::endl;
    for(;tmp_m.n_c>0;tmp_m.n_c--)
    {
        output_file<<"    component "<<tmp_m.m_components.front().name<<" "
            <<tmp_m.m_components.front().num_p<<std::endl;
        for(;tmp_m.m_components.front().num_p>0;tmp_m.m_components.front().num_p--)
        {
            output_file<<"        pin "<<tmp_m.m_components.front().pins.front().name;
            if(tmp_m.m_components.front().pins.front().msb_p==-1)
            {
                output_file<<std::endl;
                tmp_m.m_components.front().pins.pop_front();
                continue;
            }
            else if(tmp_m.m_components.front().pins.front().msb_p==tmp_m.m_components.front().pins.front().lsb_p)
            {
                output_file<<" "<<tmp_m.m_components.front().pins.front().msb_p<<std::endl;
                tmp_m.m_components.front().pins.pop_front();
            }

            else
            {
                output_file<<" "<<tmp_m.m_components.front().pins.front().msb_p<<" "
                    <<tmp_m.m_components.front().pins.front().lsb_p<<std::endl;
                tmp_m.m_components.front().pins.pop_front();
            }
        }
        tmp_m.m_components.pop_front();
    }
    return 0;
}

bool make_wires_table(const evl_wires &wires,evl_wires_table &wires_table)
{
    for (auto &wire: wires) {
        wires_table.insert(std::make_pair(wire.name, wire.width));
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool netlist::create(const evl_wires &wires,   const evl_components &comps,  const evl_wires_table &wires_table)
{
    return create_nets(wires)&& create_gates(comps, wires_table);
}

bool netlist::create_nets(const evl_wires &wires)
{
    evl_wires::const_iterator it=wires.begin();  
    for(;it!=wires.end();++it)
    {
        if (it->width== 1) {
            create_net(it->name);
        }
        else {
            for (int i = 0; i <it->width; ++i) {
                create_net(make_net_name(it->name, i));
            }
        }
    }
    return true;
}

bool netlist::create_gates(const evl_components &comps,  const evl_wires_table &wires_table )
{
    evl_components::const_iterator it = comps.begin();
    for(;it!=comps.end();++it)
    {
        create_gate(*it, wires_table);
    }
    return true;
}

void netlist::create_net(std::string net_name)
{
    assert(nets_table.find(net_name) == nets_table.end());
    net *n = new net(net_name);
    nets_table[net_name] = n;
    nets_.push_back(n);
}

bool netlist::create_gate(const evl_component &c, const evl_wires_table &wires_table) 
{
    gate *g = new gate;
    gates_.push_back(g);
    return g->create(c, nets_table, wires_table);
}

bool gate::create(const evl_component &c, const std::map<std::string, net *> &nets_table, const evl_wires_table &wires_table)
{
    //set gate type and name;
    int size=c.name.length();
    int i=0;
    for(;i<size+2;i++)
    {
        if(c.name[i]=='\0')
        {
            type_=c.name.substr(0,i);
            name_="empty";
            break;
        }
        if(c.name[i]==' ')
        {
            type_=c.name.substr(0,i);
            name_=c.name.substr(i+1,size-1);
            break;
        }
    }

    size_t index = 0;

    evl_pins::const_iterator it=c.pins.begin();
    for(;it!=c.pins.end();++it)
    {
        create_pin(*it, index, nets_table, wires_table);
        ++index;
    }
    return 1;
}

bool gate::create_pin(const evl_pin &ep, size_t index, const std::map<std::string, net *> &nets_table, const evl_wires_table &wires_table)
{
    //resolve semantics of ep using wires_table
    evl_wires_table::const_iterator it;
    if(type_=="and")
    {
        it=wires_table.find(ep.name);
        if(it==wires_table.end())
        {
            std::cout<<"no "<<ep.name<<" pin"<<std::endl;
            return false;
        }
    }

    pin *p = new pin;
    pins_.push_back(p);
    return p->create(this, index, ep, nets_table);
}

bool pin::create(gate *g, size_t index, const evl_pin &p, const std::map<std::string, net *> &nets_table)
{
    std::string net_name;
    //store g and index;
    gate_=g;
    index_=index;
    int i=0;

    name_=p.name;

    if (p.msb_p== -1) // a 1-bit wire
    { 
        net_name = p.name;
        net_ = (nets_table.find(net_name))->second;
        net_->append_pin(this);
        width=1;
    } 
    else if(p.msb_p==p.lsb_p)
    {
        net_name=make_net_name(p.name,p.msb_p);
        net_ = (nets_table.find(net_name))->second;
        net_->append_pin(this);
        lsb=p.lsb_p;
        width=1;
    }
    else  //bus
    {
        for(i=p.lsb_p;i<=p.msb_p;i++)
        {
            net_name=make_net_name(p.name,i);
            net_ = (nets_table.find(net_name))->second;
            net_->append_pin(this);
        }
        width=p.msb_p-p.lsb_p+1;
         lsb=p.lsb_p;
    }
    return true;
}

void net::append_pin(pin *p)
{
    connections_.push_back(p);
}

std::string make_net_name(std::string wire_name, int i) 
{
    assert(i >= 0);
    std::ostringstream oss;
    oss << wire_name << "[" << i << "]";
    return oss.str();
}

void netlist::display_netlist(evl_module tmp_m,std::ofstream &output_file)
{
    std::list<net *>::iterator it_n=this->nets_.begin();

    output_file<<"module "<<tmp_m.name<<std::endl;
    output_file<<"nets "<<this->nets_.size()<<std::endl;
    for(;it_n!=this->nets_.end();++it_n)
    {
        (*it_n)->display_nets(output_file);
    }

}

void net::display_nets(std::ofstream &output_file)
{
    std::list<pin *>::iterator it_p=this->connections_.begin();
    output_file<<"    net "<<this->name_<<" "<<this->connections_.size()<<std::endl;
    for(;it_p!=this->connections_.end();++it_p)
    {
        (*it_p)->display_connections(output_file);
    }
}

void pin::display_connections(std::ofstream &output_file)
{
    output_file<<"        "<<this->gate_->get_name()<<" "<<index_<<std::endl;
}

void display_comp(evl_module tmp_m,std::ofstream &output_file,const evl_wires_table &wires_table)
{
    int width_w=-1;
    int i=0;
    output_file<<"components "<<tmp_m.n_c<<std::endl;
    for(;tmp_m.n_c>0;tmp_m.n_c--)
    {
        output_file<<"    component "<<tmp_m.m_components.front().name<<" "
            <<tmp_m.m_components.front().num_p<<std::endl;
        for(;tmp_m.m_components.front().num_p>0;tmp_m.m_components.front().num_p--)
        {
            output_file<<"        pin ";
            // output_file<<"        pin "<<tmp_m.m_components.front().pins.front().name;
            if(tmp_m.m_components.front().pins.front().msb_p==-1)
            {
                width_w=wires_table.find(tmp_m.m_components.front().pins.front().name)->second;
                output_file<<width_w;   //wires width
                if(width_w==1)
                {
                    output_file<<" "<<tmp_m.m_components.front().pins.front().name;
                }
                else
                {
                    for(i=0;i<width_w;i++)
                    {
                        output_file<<" "<<tmp_m.m_components.front().pins.front().name<<"["<<i<<"]";
                    }
                }
                output_file<<std::endl;
                tmp_m.m_components.front().pins.pop_front();
                continue;
            }
            else if(tmp_m.m_components.front().pins.front().msb_p==tmp_m.m_components.front().pins.front().lsb_p)
            {
                output_file<<"1 "<<tmp_m.m_components.front().pins.front().name<<"["<<tmp_m.m_components.front().pins.front().msb_p<<"]"<<std::endl;
                tmp_m.m_components.front().pins.pop_front();
            }

            else
            {
                /* output_file<<" "<<tmp_m.m_components.front().pins.front().msb_p<<" "
                <<tmp_m.m_components.front().pins.front().lsb_p<<std::endl;*/
                width_w=tmp_m.m_components.front().pins.front().msb_p-tmp_m.m_components.front().pins.front().lsb_p+1;
                output_file<<width_w;   //pins width
                for(i=0;i<width_w;i++)
                {
                    output_file<<" "<<tmp_m.m_components.front().pins.front().name<<"["<<tmp_m.m_components.front().pins.front().lsb_p+i<<"]";
                }
                output_file<<std::endl;
                tmp_m.m_components.front().pins.pop_front();
            }
        }
        tmp_m.m_components.pop_front();
    }
}

void  undate_pin(evl_module &tmp_m,const evl_wires_table &wires_table)
{
    evl_components::iterator it_c=tmp_m.m_components.begin();
    evl_pins::iterator it_p;
    int width=1;
    for(;it_c!=tmp_m.m_components.end();++it_c)
    {
        it_p=(*it_c).pins.begin();
        for(;it_p!=(*it_c).pins.end();++it_p)
        {
            if((*it_p).msb_p==-1)
            {
                width=wires_table.find((*it_p).name)->second;
                if(width!=1)
                {
                    (*it_p).msb_p=width-1;
                    (*it_p).lsb_p=0;
                }
            }
        }
    }
}

std::string gate::get_name()
{
    if(this->name_=="empty")
        return this->type_;
    else
        return this->type_+' '+this->name_;
}
//-------------------------------------------proj4-------------------------------
void netlist::compute_next_state_and_output() {
    int i;
    for (gate *g: gates_)
    {
        g->validate_structural_semantics();
    }
    for(i=0;i<100;i++)
    {
        for (net *n: nets_)
            n->set_signal('?');
        for (gate *g: gates_)
        {
            g->compute_next_state_or_output(nets_table);
        }
         for (gate *g: gates_)
        {
            g->change_state();
        }
    }
}

void net::set_signal(char s)
{
    this->signal_=s;
}

char net::get_signal() {
    if (signal_ == '?') {
        std::list<pin*>::iterator it=connections_.begin();
        for(;it!=connections_.end();it++)
        {
            if((*it)->get_dir()=='O')
            {
                signal_= (*it)->compute_signal();
                break;
            }
        }

    }
    return signal_;
}
char pin::compute_signal() {
    if (dir_ == 'O')
        return gate_->compute_signal(index_);
    else // dir_ == 'I'
        return net_->get_signal();
}
void gate::compute_next_state_or_output(std::map<std::string, net *> nets_table)
{
    if (type_ == "evl_dff") 
    {
        //next_state_ = (pins_[1])->compute_signal(); // d
        std::list<pin*>::iterator it=pins_.begin();
        it++;
        next_state_=(*it)->compute_signal();
    }
    else if (type_ == "evl_output")
    {
        std::list<pin*>::iterator it=pins_.begin();
        int j,sum=0,ct;
        std::string pin_b;
        std::list<char> p_i;
        std::list<char> out_;
        std::string tmp;
        char tmp_c;
        std::string output_file_name = filename_+'.'+name_+".evl_output";
        std::ofstream output_file(output_file_name,std::ios::app);

        for(it=pins_.begin();it!=pins_.end();it++)  
        {
            if((*it)->get_width()==1)
            {
                output_file<<(*it)->compute_signal()<<' ';
                std::cout<<(*it)->compute_signal()<<' ';
            }
            else
            {
                for(j=(*it)->get_lsb();j<(*it)->get_lsb()+(*it)->get_width();j++)//accoding to the name to find the specific pin by using nets_table
                {
                    pin_b=make_net_name((*it)->get_name(),j);
                    std::map<std::string, net *> ::iterator it_t=nets_table.find(pin_b);
                    if(it_t==nets_table.end())
                    {
                        std::cout<<"wrong";
                    }
                    p_i.push_back(it_t->second->get_signal());
                }
                for(j=(*it)->get_lsb(),ct=0,sum=0;j<(*it)->get_lsb()+(*it)->get_width();j++,ct++)
                {
                    if(ct==4)
                    {
                        if(sum>=10)
                        {
                            tmp_c='A'+(sum-10);
                            out_.push_front(tmp_c);

                        }
                        else
                        {
                            tmp_c='0'+sum;
                            out_.push_front(tmp_c);
                        }
                        sum=0;
                        ct=0;
                    }
                    tmp=p_i.front();
                    sum=sum+atoi(tmp.c_str())*pow(2,ct);
                    p_i.pop_front();
                }
                if(sum>=10)
                {
                    tmp_c='A'+(sum-10);
                    out_.push_front(tmp_c);
                }
                else
                {
                    tmp_c='0'+sum;
                    out_.push_front(tmp_c);
                }
                std::list<char>::iterator it_=out_.begin();
                for(;it_!=out_.end();it_++)
                {
                    tmp_c=(*it_);
                    output_file<<tmp_c;
                    std::cout<<tmp_c;
                }
                output_file<<' ';
                std::cout<<' ';
                out_.clear();
            }
        }
        output_file<<std::endl;
    }
    else if(type_ == "evl_input")
    {
        std::list<pin*>::iterator it=pins_.begin();
        std::list<std::string> line_;
        std::list<std::string> *tmp_l=&line_;
        get_input(line_,input_.front());
        int j;
        std::string pin_b;

        if(trans==0)
        {
            trans=atoi(line_.front().c_str());
        }
        line_.pop_front(); // pop num_of_transations
       for(;it!=pins_.end();it++)
        {
            /*if(line_.front().size==1)
            {
                if()
            }*/
            if((*it)->get_width()==1)
            {
            }
            else
            {
                for(j=(*it)->get_lsb();j<(*it)->get_lsb()+(*it)->get_width();j++)
                {
                    pin_b=make_net_name((*it)->get_name(),j);
                    std::map<std::string, net *> ::iterator it_t=nets_table.find(pin_b);
                    if(it_t==nets_table.end())
                    {
                        std::cout<<"wrong";
                    }
                    
                }
            }
       }
    }
}
char gate::compute_signal(int pin_index) {
    if (type_ == "evl_dff") {
        //assert pin_index == 0; // must be q
        assert (pin_index == 0);
        return state_;
    }
    else if (type_ == "evl_zero") {
        return '0';
    }
    else if (type_ == "evl_one") {
        return '1';
    }
    else if (type_ == "and") {
        assert (pin_index == 0); // must be out
        //collect signals from the input pins
        // compute and return the output signal
        std::list<pin*>::iterator it=pins_.begin();
        it++;
        for(;it!=pins_.end();it++)
        {
            if((*it)->compute_signal()=='0')
                return '0';
        }
        return '1';
    }
     else if (type_ == "or") {
        assert (pin_index == 0); // must be out
        std::list<pin*>::iterator it=pins_.begin();
        it++;
        for(;it!=pins_.end();it++)
        {
            if((*it)->compute_signal()=='1')
                return '1';
        }
        return '0';
    }
     else if (type_ == "xor") {
        assert (pin_index == 0); // must be out
        int tmp=0;
        std::list<pin*>::iterator it=pins_.begin();
        it++;
        for(;it!=pins_.end();it++)
        {
            if((*it)->compute_signal()=='1')
                tmp++;
        }
        if(tmp%2==0)
            return '0';
        else
            return '1';
    }
     else if (type_ == "not") {
        assert (pin_index == 0); 
        std::list<pin*>::iterator it=pins_.begin();
        it++;
         if((*it)->compute_signal()=='1')
             return '0';
         else if((*it)->compute_signal()=='0')
             return '1';
    }
     else if (type_ == "buf") {
        assert (pin_index == 0); 
        std::list<pin*>::iterator it=pins_.begin();
        it++;
         if((*it)->compute_signal()=='1')
             return '1';
         else if((*it)->compute_signal()=='0')
             return '0';
    }

}
bool gate::validate_structural_semantics() {
    if (type_ == "and"||type_ == "xor"||type_ == "or") {
        if (pins_.size() < 3) 
            return false;
        // (pins_.begin())->set_as_output(); // out
        //for (size_t i = 1; i < pins_.size(); ++i)
        //pins_[i]->set_as_input();
        std::list<pin*>::iterator it=pins_.begin();
        (*it)->set_as_output();
        it++;
        for(;it!=pins_.end();it++)
        {
            (*it)->set_as_input();
        }
    }
    else if(type_ == "not"||type_ == "buf")
    {
        if (pins_.size() != 2) 
            return false;
        std::list<pin*>::iterator it=pins_.begin();
        (*it)->set_as_output();
        it++;
        (*it)->set_as_input();
    }
    else if (type_ == "evl_zero"||type_ == "evl_one") {
        std::list<pin*>::iterator it=pins_.begin();
        for(;it!=pins_.end();it++)
        {
            (*it)->set_as_output();
        }
    }
    else if (type_ == "tris") {
        if (pins_.size() != 3) 
            return false;
        std::list<pin*>::iterator it=pins_.begin();
        (*it)->set_as_output();
        it++;
        for(;it!=pins_.end();it++)
        {
            (*it)->set_as_input();
        }
    }
    else if (type_ == "evl_clock") {
        std::list<pin*>::iterator it=pins_.begin();
        (*it)->set_as_output();
     }
     else if (type_ == "evl_output") {
        std::list<pin*>::iterator it_=pins_.begin();
        for(;it_!=pins_.end();it_++)
        {
            (*it_)->set_as_input();
        }

        std::string output_file_name = filename_+'.'+name_+".evl_output";
        std::ofstream output_file(output_file_name);
        output_file<<pins_.size()<<std::endl;
        std::list<pin*>::iterator it=pins_.begin();
        for(;it!=pins_.end();it++)
        {
            output_file<<(*it)->get_width()<<std::endl;
        }

     }
     else if (type_ == "evl_input") {
         std::list<pin*>::iterator it_=pins_.begin();
        for(;it_!=pins_.end();it_++)
        {
            (*it_)->set_as_output();
        }
        std::string input_file_name = filename_+'.'+name_+".evl_input";
        std::ifstream input_file(input_file_name);
        if (!input_file)
        {
            std::cerr << "I can't read " <<input_file_name << "." << std::endl;
            return -1;
        }
        std::string line;
         for (int line_no = 1; std::getline(input_file, line); ++line_no)
         {
             input_.push_back(line);
         }
        std::list<std::string> line_;
        std::list<std::string> *tmp_l=&line_;
        get_input(line_,input_.front());

        if(pins_.size()!=atoi(line_.front().c_str()))
        {
            std::cout<<"wrong pin number";
        }
        input_.pop_front();
        trans=0;
     }
     else if (type_ == "evl_dff") {
         if (pins_.size() != 3) 
             return false;
         std::list<pin*>::iterator it=pins_.begin();
         (*it)->set_as_output();
         it++;
         for(;it!=pins_.end();it++)
         {
             (*it)->set_as_input();
         }
         state_='0';
     }
}
void gate::change_state()
{
    if(type_=="evl_dff")
        state_=next_state_;
   // if(type_=="evl_input")
}
void pin::set_as_input()
{
    this->dir_='I';
}
void pin::set_as_output()
{
    this->dir_='O';
}
int pin::get_width()
{
    return width;
}
int pin::get_lsb()
{
    return lsb;
}
char pin::get_dir()
{
    return dir_;
}
std::string pin::get_name()
{
    return name_;
}
void get_input(std::list<std::string> &line_,std::string line_s)
{
    std::string::iterator it_c=line_s.begin();
    std::string tmp_s;
    int i=0;
    for(;it_c!=line_s.end();it_c++)
    {
        if((*it_c)==' ')
        {
            line_.push_back(tmp_s);
            tmp_s.clear();
            continue;
        }
        tmp_s=tmp_s+(*it_c);
    }
    line_.push_back(tmp_s);
    tmp_s.clear();
}