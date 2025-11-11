#include "json_format_io.hpp"

#include <iomanip>
#include <stack>

namespace SYNC_LIB
{
    json_format_io::json_format_io(void)
    {
    }

    json_format_io::~json_format_io(void)
    {
    }

    void json_format_io::write(ostream &os, const string &instance_name, const sync_scheduling &schedules) const
    {
        // os << "{" << endl;
        // os << "  \"instance_name\": \"" << instance_name << "\"," << endl;
        // os << "  \"obj_value\": " << obj_value << "," << endl;
        os << "  \"schedule\": " << endl;

        // write_vector_of_double_vectors(os, schedules);
        write_vector_of_scheduling(os, schedules);
        os << endl;
        // os << "}" << endl;
    }

    void json_format_io::write_sol(ostream &os, const string &instance_name, const vector<vector<int>> &routes) const
    {
        write_vector_of_integer_vectors(os, routes);
    }

    char json_format_io::read_next_non_space_char(istream &is) const
    {
        char ch;
        do
        {
            is.get(ch);
        } while (isspace(ch) && is);

        return ch;
    }

    void json_format_io::read_integer_vector(istream &is, vector<int> &vec) const
    {
        char ch = read_next_non_space_char(is);
        if (ch != '[')
        {
            cerr << "Error: Expected '[' at the beginning of integer vector." << endl;
            return;
        }

        while (true)
        {
            int value;
            is >> value;
            vec.push_back(value);

            ch = read_next_non_space_char(is);
            if (ch == ']')
            {
                break;
            }
            else if (ch != ',')
            {
                cerr << "Error: Expected ',' or ']' in integer vector." << endl;
                return;
            }
        }
    }

    void json_format_io::read_double_vector(istream &is, vector<double> &vec) const
    {
        char ch = read_next_non_space_char(is);
        if (ch != '[')
        {
            cerr << "Error: Expected '[' at the beginning of double vector." << endl;
            return;
        }

        while (true)
        {
            double value;
            is >> value;
            vec.push_back(value);

            ch = read_next_non_space_char(is);
            if (ch == ']')
            {
                break;
            }
            else if (ch != ',')
            {
                cerr << "Error: Expected ',' or ']' in double vector." << endl;
                return;
            }
        }
    }

    void json_format_io::write_integer_vector(ostream &os, const vector<int> &vec) const
    {
        os << "    [ ";
        for (size_t i = 0; i < vec.size(); ++i)
        {
            os << setw(3) << vec[i];

            if (i < vec.size() - 1)
            {
                os << ", ";
            }
        }
        os << " ]";
    }

    void json_format_io::write_double_vector(ostream &os, const vector<double> &vec) const
    {
        os << "    [ ";
        for (size_t i = 0; i < vec.size(); ++i)
        {
            os << setw(6) << fixed << setprecision(1) << vec[i];
            if (i < vec.size() - 1)
            {
                os << ", ";
            }
        }
        os << " ]";
    }

    bool json_format_io::read_vector_of_integer_vectors(istream &is, vector<vector<int>> &vec) const
    {
        char ch = read_next_non_space_char(is);
        if (ch != '[')
        {
            return false;
        }

        while (true)
        {
            vector<int> inner_vec;
            read_integer_vector(is, inner_vec);
            vec.push_back(inner_vec);

            ch = read_next_non_space_char(is);
            if (ch == ']')
            {
                break;
            }
            else if (ch != ',')
            {
                cerr << "Error: Expected ',' or ']' in vector of integer vectors." << endl;
                return false;
            }
        }

        return true;
    }

    bool json_format_io::read_vector_of_double_vectors(istream &is, vector<vector<double>> &vec) const
    {
        char ch = read_next_non_space_char(is);
        if (ch != '[')
        {
            return false;
        }

        while (true)
        {
            vector<double> inner_vec;
            read_double_vector(is, inner_vec);
            vec.push_back(inner_vec);

            ch = read_next_non_space_char(is);
            if (ch == ']')
            {
                break;
            }
            else if (ch != ',')
            {
                cerr << "Error: Expected ',' or ']' in vector of double vectors." << endl;
                return false;
            }
        }

        return true;
    }

    void json_format_io::write_vector_of_integer_vectors(ostream &os, const vector<vector<int>> &vec) const
    {
        os << "  [";
        os << endl;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            write_integer_vector(os, vec[i]);
            if (i < vec.size() - 1)
            {
                os << ", ";
            }
            os << endl;
        }
        os << "  ]";
    }

    void json_format_io::write_vector_of_double_vectors(ostream &os, const vector<vector<double>> &vec) const
    {
        os << "  [" << endl;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            write_double_vector(os, vec[i]);
            if (i < vec.size() - 1)
            {
                os << ", ";
            }
            os << endl;
        }
        os << "  ]";
    }

    void json_format_io::write_vector_of_scheduling(ostream &os, const vector<vector<operation_info>> &vec) const
    {
        os << "  [" << endl;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            os << "  {" << endl;
            os << "    \"route\": " << i + 1 << "," << endl;
            os << "    \"tasks\": " << endl;
            os << "    [" << endl;
            for (size_t j = 0; j < vec[i].size(); ++j)
            {
                const operation_info &op_info = vec[i][j];

                os << "      { \"customer\": " << setw(3) << op_info.first
                   << ", \"arrival_starting\": [" << setw(6) << fixed << setprecision(1) << op_info.second.first << ", " << setw(6) << fixed << setprecision(1) << op_info.second.second << "] }";
                if (j < vec[i].size() - 1)
                {
                    os << ",";
                }
                os << endl;
            }
            os << "    ]" << endl;
            os << "  }";

            if (i < vec.size() - 1)
            {
                os << ",";
            }
            os << endl;
        }
        os << "  ]" << endl;
    }

    // void json_format_io::read_sch(istream &is, string &instance_name, vector<vector<double>> &schedules)
    // {
    //     string line;
    //     getline(is, line); // {

    //     getline(is, line); // "instance_name": "name",

    //     {
    //         const size_t pos1 = line.find(":");
    //         const size_t pos2 = line.find_last_of("\"");
    //         string read_instance_name = line.substr(pos1 + 3, pos2 - pos1 - 3);
    //         instance_name = read_instance_name;
    //     }

    //     // getline(is, line); // "obj_value": value,
    //     // {
    //     //     const size_t pos1 = line.find(":");
    //     //     const size_t pos2 = line.find_last_of(",");
    //     //     string obj_value_str = line.substr(pos1 + 2, pos2 - pos1 - 2);
    //     //     obj_value = stod(obj_value_str);
    //     // }

    //     getline(is, line); // "schedules": [

    //     // Find [
    //     {
    //         const size_t pos = line.find("[");
    //         if (pos == string::npos)
    //         {
    //             cerr << "Error: Expected '[' for schedules." << endl;
    //             return;
    //         }
    //     }

    //     schedules.clear();

    //     bool done{false};

    //     while (!done)
    //     {
    //         vector<double> schedule;

    //         read_double_vector(is, schedule);

    //         schedules.push_back(schedule);

    //         char ch = read_next_non_space_char(is);
    //         if (ch == ']')
    //         {
    //             done = true;
    //         }
    //         else if (ch != ',')
    //         {
    //             cerr << "Error: Expected ',' or ']' after schedule." << endl;
    //             return;
    //         }
    //     }
    // }

    void json_format_io::read_sol(istream &is, string &instance_name, vector<vector<int>> &routes) const
    {
        string line;
        getline(is, line); // {

        getline(is, line); // "instance_name": "name",

        {
            const size_t pos1 = line.find(":");
            const size_t pos2 = line.find_last_of("\"");
            string read_instance_name = line.substr(pos1 + 3, pos2 - pos1 - 3);
            instance_name = read_instance_name;
        }

        // getline(is, line); // "obj_value": value,
        // {
        //     const size_t pos1 = line.find(":");
        //     const size_t pos2 = line.find_last_of(",");
        //     string obj_value_str = line.substr(pos1 + 2, pos2 - pos1 - 2);
        //     obj_value = stod(obj_value_str);
        // }

        getline(is, line); // "routes": [

        // Find [
        {
            const size_t pos = line.find("[");
            if (pos == string::npos)
            {
                cerr << "Error: Expected '[' for routes." << endl;
                return;
            }
        }

        routes.clear();

        bool done{false};

        while (!done)
        {
            vector<int> route;

            read_integer_vector(is, route);

            routes.push_back(route);

            char ch = read_next_non_space_char(is);
            if (ch == ']')
            {
                done = true;
            }
            else if (ch != ',')
            {
                cerr << "Error: Expected ',' or ']' after route." << endl;
                return;
            }
        }
    }

    void json_format_io::write_pair_(ostream &os, const pair<double, double> &p) const
    {
        os << "\"tw\": [" << setw(6) << fixed << setprecision(1) << p.first << ", " << setw(6) << fixed << setprecision(1) << p.second << "]";
    }

    void json_format_io::read_pair_(istream &is, pair<double, double> &p) const
    {
        char ch = read_next_non_space_char(is);
        if (ch != '[')
        {
            cerr << "Error: Expected '[' at the beginning of pair." << endl;
            return;
        }

        is >> p.first;

        ch = read_next_non_space_char(is);
        if (ch != ',')
        {
            cerr << "Error: Expected ',' in pair." << endl;
            return;
        }

        is >> p.second;

        ch = read_next_non_space_char(is);
        if (ch != ']')
        {
            cerr << "Error: Expected ']' at the end of pair." << endl;
            return;
        }
    }

    void json_format_io::write_vector_of_pairs(ostream &os, const vector<pair<double, double>> &vec) const
    {
        os << "  [";
        os << endl;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            os << "      { ";
            os << "\"customer\": " << setw(3) << i + 1 << ", ";
            write_pair_(os, vec[i]);
            os << " }";

            if (i < vec.size() - 1)
            {
                os << ", ";
            }

            os << endl;
        }
        os << "  ]" << endl;
    }

    void json_format_io::read_vector_of_pairs(istream &is, vector<pair<double, double>> &vec) const
    {
        char ch = read_next_non_space_char(is);
        if (ch != '[')
        {
            return;
        }

        // while (true)
        // {
        //     pair<int, int> p;
        //     read_pair_(is, p);
        //     vec.push_back(p);

        //     ch = read_next_non_space_char(is);
        //     if (ch == ']')
        //     {
        //         break;
        //     }
        //     else if (ch != ',')
        //     {
        //         cerr << "Error: Expected ',' or ']' in vector of pairs." << endl;
        //         return;
        //     }
        // }
    }
}