#include "TSPLIB_instance.hpp"

#include <cstring>
#include <cmath>
#include <fstream>
#include <cstdio>

#include <stdlib.h>
#include <errno.h>

#define LENGTH 255

#define LINE_LEN 80

#define _EXPLICIT 0
#define _EUC_2D 1
#define _EUC_3D 2
#define _MAX_2D 3
#define _MAX_3D 4
#define _MAN_2D 5
#define _MAN_3D 6
#define _CEIL_2D 7
#define _GEO 8
#define _ATT 9

#define TSPLIB_PI 3.141592

namespace TSP
{

    TSPLIB_instance::TSPLIB_instance(void) : name_(), type_(), comment_(), dimension_(-1), edge_weight_type_(-1), edge_weight_format_(-1), display_data_type_(-1), num_days_(-1), max_distance_(-1), maximum_allowable_differencial_(-1), depot_(-1), distances_(NULL), coord_id_(), coord_(), display_id_(), display_(), demand_()
    {
        distance_function_.resize(WTYPE_NUM);
        read_function_.resize(KEY_NUM);
        edge_weight_reading_function_.resize(WFORMAT_NUM);

        distance_function_[_EXPLICIT] = NULL;
        distance_function_[_EUC_2D] = &TSPLIB_instance::compute_euc_2d_distance_;
        distance_function_[_EUC_3D] = NULL;
        distance_function_[_MAX_2D] = &TSPLIB_instance::compute_max_2d_distance_;
        distance_function_[_MAX_3D] = NULL;
        distance_function_[_MAN_2D] = &TSPLIB_instance::compute_man_2d_distance_;
        distance_function_[_MAN_3D] = NULL;
        distance_function_[_CEIL_2D] = &TSPLIB_instance::compute_ceil_2d_distance_;
        distance_function_[_GEO] = &TSPLIB_instance::compute_geo_distance_;
        distance_function_[_ATT] = &TSPLIB_instance::compute_att_distance_;

        read_function_[0] = &TSPLIB_instance::read_name_section_;
        read_function_[1] = &TSPLIB_instance::read_type_section_;
        read_function_[2] = &TSPLIB_instance::read_comment_section_;
        read_function_[3] = &TSPLIB_instance::read_dimension_section_;
        read_function_[4] = &TSPLIB_instance::read_capacity_section_;
        read_function_[5] = &TSPLIB_instance::read_edge_weight_type_section_;
        read_function_[6] = &TSPLIB_instance::read_edge_weigh_format_section_;
        read_function_[7] = &TSPLIB_instance::read_display_data_type_section_;
        read_function_[8] = &TSPLIB_instance::read_edge_weight_section_;
        read_function_[9] = &TSPLIB_instance::read_display_data_section_;
        read_function_[10] = &TSPLIB_instance::read_node_coord_section_;
        read_function_[11] = &TSPLIB_instance::read_node_coord_type_section_;
        read_function_[12] = &TSPLIB_instance::read_depot_section_;
        read_function_[13] = &TSPLIB_instance::read_capacity_vol_section_;
        read_function_[14] = &TSPLIB_instance::read_demand_section_;
        read_function_[15] = &TSPLIB_instance::read_time_window_section_;
        read_function_[16] = &TSPLIB_instance::read_standtime_section_;
        read_function_[17] = &TSPLIB_instance::read_pickup_section_;
        read_function_[18] = &TSPLIB_instance::read_EOF_section_;
        read_function_[19] = &TSPLIB_instance::read_number_of_trucks_section_;
        read_function_[20] = &TSPLIB_instance::read_num_days_section_;
        read_function_[21] = &TSPLIB_instance::read_distance_section_;
        read_function_[22] = &TSPLIB_instance::read_maximum_allowable_differencial_section_;

        edge_weight_reading_function_[0] = &TSPLIB_instance::read_edge_weight_section_upper_row_;
        edge_weight_reading_function_[1] = &TSPLIB_instance::read_edge_weight_section_lower_row_;
        edge_weight_reading_function_[2] = &TSPLIB_instance::read_edge_weight_section_upper_diag_row_;
        edge_weight_reading_function_[3] = &TSPLIB_instance::read_edge_weight_section_lower_diag_row_;
        edge_weight_reading_function_[4] = &TSPLIB_instance::read_edge_weight_section_upper_col_;
        edge_weight_reading_function_[5] = &TSPLIB_instance::read_edge_weight_section_lower_col_;
        edge_weight_reading_function_[6] = &TSPLIB_instance::read_edge_weight_section_upper_diag_col_;
        edge_weight_reading_function_[7] = &TSPLIB_instance::read_edge_weight_section_lower_diag_col_;
        edge_weight_reading_function_[8] = &TSPLIB_instance::read_edge_weight_section_full_matrix_;
    }

    TSPLIB_instance::~TSPLIB_instance(void)
    {
        if (distances_ != NULL)
            delete[] distances_;
    }

    void TSPLIB_instance::get_distances(GOMA::matrix<double> &distances) const
    {

        distances.resize(dimension_, dimension_);

        for (int i = 0; i < dimension_; i++)
            for (int j = 0; j < dimension_; j++)
                if (i != j)
                    distances(i + 1, j + 1) = distances_[i * dimension_ + j];
                else
                    distances(i + 1, j + 1) = 100000000.0;
                
    }

    void TSPLIB_instance::read_name_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "File                          : " << s_token << endl;
        name_ = s_token;
    }

    void TSPLIB_instance::read_type_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "Type                          : " << s_token << endl;
        type_ = s_token;
    }

    void TSPLIB_instance::read_dimension_section_(istream &is, ostream &os)
    {
        is >> dimension_;
        establish_dimension_(dimension_);
        os << "Dimension                     : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_type_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "Edge Weigh Type               : " << s_token << endl;
        edge_weight_type_ = find_key_(s_token, wtypes);
    }

    void TSPLIB_instance::read_edge_weigh_format_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "Edge Weigh Format             : " << s_token << endl;
        edge_weight_format_ = find_key_(s_token, wformats);
    }

    void TSPLIB_instance::read_display_data_type_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "Display Data Type             : " << s_token << endl;
        display_data_type_ = find_key_(s_token, dtypes);
    }

    void TSPLIB_instance::read_depot_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> depot_;
        os << "Depot                         : " << depot_ << endl;
        is >> s_token;
    }

    void TSPLIB_instance::read_num_days_section_(istream &is, ostream &os)
    {
        is >> num_days_;
        os << "Number of days                : " << num_days_ << endl;
    }

    void TSPLIB_instance::read_distance_section_(istream &is, ostream &os)
    {
        is >> max_distance_;
        os << "Distance                      : " << max_distance_ << endl;
    }

    void TSPLIB_instance::read_maximum_allowable_differencial_section_(istream &is, ostream &os)
    {
        is >> maximum_allowable_differencial_;
        os << "Maximum allowable differential: " << maximum_allowable_differencial_ << endl;
    }

    void TSPLIB_instance::establish_dimension_(const int dimension)
    {

        if (distances_ != NULL)
            delete[] distances_;

        distances_ = NULL;

        distances_ = new double[dimension * dimension];

        for (int i = 0; i < dimension_; i++)
            for (int j = 0; j < dimension_; j++)
                distances_[i * dimension_ + j] = 0.0;

        coord_id_.resize(dimension);
        coord_.resize(dimension);

        display_id_.resize(dimension);
        display_.resize(dimension);

        demand_.resize(dimension);
    }

    void TSPLIB_instance::read_display_data_section_(istream &is, ostream &os)
    {
        if ((display_data_type_ == -1) || (display_data_type_ == 2))
        {
            cerr << "Display data type not defined" << endl;
            exit(1);
        }

        int num;
        double x, y;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> x;
            is >> y;

            display_id_[num - 1] = num;
            display_[num - 1] = coordType(x, y);
        }

        os << "Reading coords                : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_demand_section_(istream &is, ostream &os)
    {
        if (num_days_ == -1)
        {
            cerr << "Number of days not defined" << endl;
            exit(1);
        }

        int num, d;

        for (size_t i = 0; i < demand_.size(); i++)
            demand_[i].resize(num_days_);

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;

            vector<int> &demand_i{demand_[num - 1]};

            for (int j = 0; j < num_days_; j++)
            {
                is >> d;
                demand_i[j] = d;
            }
        }
        os << "Reading demands               : " << demand_.size() << endl;        
    }

    void TSPLIB_instance::read_time_window_section_(istream &is, ostream &os)
    {
        int num, a, b;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> a;
            is >> b;
        }

        os << "Reading time windows          : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_standtime_section_(istream &is, ostream &os)
    {
        int num, s;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> s;
        }

        os << "Reading standtimes            : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_pickup_section_(istream &is, ostream &os)
    {
        int num, p;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> p;
        }

        os << "Reading pickups               : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_EOF_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "EOF                           : " << s_token << endl;
        os << endl;
    }

    void TSPLIB_instance::read_capacity_vol_section_(istream &is, ostream &os)
    {
        int num, c;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> c;
        }

        os << "Reading capacity volumes      : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_node_coord_type_section_(istream &is, ostream &os)
    {
        string s_token;

        is >> s_token;
        os << "Node Coord Type               : " << s_token << endl;
    }

    void TSPLIB_instance::read_number_of_trucks_section_(istream &is, ostream &os)
    {
        int num;

        is >> num;
        os << "Number of trucks              : " << num << endl;
    }

    void TSPLIB_instance::read_capacity_section_(istream &is, ostream &os)
    {
        int num, c;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> c;
        }

        os << "Reading capacities            : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_node_coord_section_(istream &is, ostream &os)
    {
        if (edge_weight_type_ == -1)
        {
            cerr << "Edge weight type not defined" << endl;
            exit(1);
        }
        else if (edge_weight_type_ == _EXPLICIT)
        {
            cerr << "Edge weight type is explicit" << endl;
            exit(1);
        }

        int num;
        double x, y;

        for (int i = 0; i < dimension_; i++)
        {
            is >> num;
            is >> x;
            is >> y;

            coord_id_[i] = num;
            coord_[i] = coordType(x, y);
        }

        if (edge_weight_type_ != _EXPLICIT)
            compute_implicit_distance_matrix_();

        os << "Reading coords                : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_upper_row_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{i + 1}; j < dimension_; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_lower_row_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j < i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_upper_diag_row_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j <= i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_lower_diag_row_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j <= i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_upper_col_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j < i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_lower_col_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j < i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_upper_diag_col_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j <= i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_lower_diag_col_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j <= i; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
                distances_[j * dimension_ + i] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_full_matrix_(istream &is, ostream &os)
    {
        int k;

        for (int i{0}; i < dimension_; i++)
            for (int j{0}; j < dimension_; j++)
            {
                is >> k;
                distances_[i * dimension_ + j] = k;
            }

        os << "Reading distances             : " << dimension_ << endl;
    }

    void TSPLIB_instance::read_edge_weight_section_(istream &is, ostream &os)
    {

        if (edge_weight_type_ == -1)
        {
            cerr << "Edge weight type not defined" << endl;
            exit(1);
        }

        if (edge_weight_format_ == -1)
        {
            cerr << "Edge weight format not defined" << endl;
            exit(1);
        }

        if (edge_weight_type_ != _EXPLICIT)
        {
            cerr << "Edge weight type is not explicit" << endl;
            exit(1);
        }

        (this->*edge_weight_reading_function_[edge_weight_format_])(is, os);
    }

    void TSPLIB_instance::compute_implicit_distance_matrix_(void)
    {

        if (edge_weight_type_ == -1)
        {
            cerr << "Edge weight type not defined" << endl;
            exit(1);
        }

        if (edge_weight_type_ == _EXPLICIT)
        {
            cerr << "Edge weight type is explicit" << endl;
            exit(1);
        }

        for (int i = 0; i < dimension_; i++)
            for (int j = 0; j < dimension_; j++)
            {
                const coordType &coord_i{coord_[i]};
                const coordType &coord_j{coord_[j]};

                const double distance_ij{(this->*distance_function_[edge_weight_type_])(coord_i, coord_j)};

                distances_[i * dimension_ + j] = distance_ij;
            }
    }

    int TSPLIB_instance::nint_(double x) const
    {

        return (int)(x + 0.5);
    }

    double TSPLIB_instance::dtrunc_(double x) const
    {
        const int k{(int)x};
        const double y{(double)k};

        return y;
    }

    void TSPLIB_instance::radian_coords_(const coordType &a, double &longitude, double &latitude) const
    {

        double x{a.first};

        const double deg_x{dtrunc_(x)};
        const double min_x{x - deg_x};

        latitude = TSPLIB_PI * (deg_x + 5.0 * min_x / 3.0) / 180.0;

        double y{a.second};

        const double deg_y{dtrunc_(y)};
        const double min_y{y - deg_y};

        longitude = TSPLIB_PI * (deg_y + 5.0 * min_y / 3.0) / 180.0;
    }

    double TSPLIB_instance::compute_euc_2d_distance_(const coordType &a, const coordType &b) const
    {

        const double xd{a.first - b.first};
        const double yd{a.second - b.second};

        return nint_(sqrt(xd * xd + yd * yd));
    }

    double TSPLIB_instance::compute_max_2d_distance_(const coordType &a, const coordType &b) const
    {

        const double xd{fabs(a.first - b.first)};
        const double yd{fabs(a.second - b.second)};

        return max(nint_(xd), nint_(yd));
    }

    double TSPLIB_instance::compute_man_2d_distance_(const coordType &a, const coordType &b) const
    {

        const double xd{fabs(a.first - b.first)};
        const double yd{fabs(a.second - b.second)};

        return nint_(xd + yd);
    }

    double TSPLIB_instance::compute_ceil_2d_distance_(const coordType &a, const coordType &b) const
    {

        const double xd{a.first - b.first};
        const double yd{a.second - b.second};

        return ceil(sqrt(xd * xd + yd * yd));
    }

    double TSPLIB_instance::compute_geo_distance_(const coordType &a, const coordType &b) const
    {

        double longitude_a, latitude_a;
        double longitude_b, latitude_b;

        radian_coords_(a, longitude_a, latitude_a);
        radian_coords_(b, longitude_b, latitude_b);

        const double RRR{6378.388};

        const double long_diffab{longitude_a - longitude_b};
        const double latt_diffab{latitude_a - latitude_b};
        const double latt_sum_ab{latitude_a + latitude_b};

        const double q1{cos(long_diffab)};
        const double q2{cos(latt_diffab)};
        const double q3{cos(latt_sum_ab)};

        const double acos_arg{0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)};

        return (int)(RRR * acos(acos_arg) + 1.0);
    }

    double TSPLIB_instance::compute_att_distance_(const coordType &a, const coordType &b) const
    {
        const double xd{a.first - b.first};
        const double yd{a.second - b.second};

        const double rij{sqrt((xd * xd + yd * yd) / 10.0)};

        const double tij{(double)nint_(rij)};

        return (tij < rij) ? tij + 1.0 : tij;
    }

    void TSPLIB_instance::read_comment_section_(istream &is, ostream &os)
    {
        optimal_values_.resize(2);

        is >> optimal_values_[0];

        char c_token{0};

        while ((c_token != ',') && (c_token != '\n') && !is.eof())
            is >> c_token;

        is >> optimal_values_[1];

        string s_token;

        getline(is, s_token);

        os << "Comment                       : " << "Optimal value not allowing waiting: " << setw(5) << optimal_values_[0] << endl;
        os << "                                Optimal value allowing waiting    : " << setw(5) << optimal_values_[1] << endl;
    }

    void TSPLIB_instance::read(const string &input_file)
    {
        const string temp_file{input_file + ".tmp"};

        clean_(input_file, temp_file);

        ifstream is(temp_file);

        read_(is, cout);

        is.close();

        remove(temp_file.c_str());
    }

    void TSPLIB_instance::clean_(const string &i_file, const string &o_file)
    {
        ifstream is(i_file);

        if (!is)
        {
            cerr << "ERROR opening input file:";
            cerr << i_file << endl;
            exit(1);
        }

        ofstream os(o_file);

        clean_(is, os);

        is.close();
        os.close();
    }

    void TSPLIB_instance::clean_(istream &is, ostream &os)
    {
        string line;

        while (getline(is, line))
        {
            std::string::iterator end_pos = std::remove(line.begin(), line.end(), ':');
            line.erase(end_pos, line.end());

            os << line << endl;
        }
    }

    int TSPLIB_instance::find_key_(const string &token, const vector<string> keywrds) const
    {
        const size_t key_num{keywrds.size()};

        for (size_t i = 0; i < key_num; i++)
            if (token == keywrds[i])
                return i;

        return -1;
    }

    void TSPLIB_instance::read_(istream &is, ostream &os)
    {

        os << messages[0] << endl;

        while (!is.eof())
        {
            string s_token;

            is >> s_token;

            const int k{find_key_(s_token, keywords)};

            (this->*read_function_[k])(is, os);
        }
    }
}