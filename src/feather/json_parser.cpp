#include "json_parser.h"
#include "utility.h"
#include <cstddef>
#include <cstring>
#include <fstream>
#include <math.h>

#include <stdlib.h>
#include <string>
#include <vector>
namespace feather
{
    template <>
    int json_value::get_as<int>()
    {
        if (type == JSON_INT)
        {
            return std::stoi(vdata, nullptr);
        }
        else
        {
            context.generate_error("error, trying to get a invalid json value type");
            return -1;
        }
    };
    template <>
    std::string json_value::get_as<std::string>()
    {
        if (type == JSON_STRING)
        {
            char *data = new char[strlen(vdata)+2];
            std::memcpy(data, vdata + 1, strlen(vdata )- 2);
            data[strlen(vdata)] = 0;
            std::string ret = data;
            delete[] data;
            return ret;
        }
        else
        {
            context.generate_error("error, trying to get a invalid json value type it is %i", type);
            return std::string("null");
        }
    };
    template <>
    float json_value::get_as<float>()
    {
        if (type == JSON_FLOAT)
        {
            return std::stof(vdata, nullptr);
        }
        else
        {
            context.generate_error("error, trying to get a invalid json value type");
            return NAN;
        }
    };
    template <>
    bool json_value::get_as<bool>()
    {
        if (type == JSON_BOOL)
        {
            if (strcmp(vdata, "true") == 0)
            {
                return true;
            }
            return false;
        }
        else
        {
            context.generate_error("error, trying to get a invalid json value type");
            return false;
        }
    };
    void json_value::detect_type()
    {
        type = JSON_NULL;
        for (int i = 0; i < strlen(vdata); i++)
        {
            if (vdata[i] == '"')
            {
                type = JSON_STRING;
                break;
            }
            if (vdata[i] == '.')
            {
                type = JSON_FLOAT;
                break;
            }
        }
        if (type == JSON_NULL)
        {
            if (strcmp(vdata, "true") == 0 || strcmp(vdata, "false") == 0)
            {

                type = JSON_BOOL;
            }
            else if (strcmp(vdata, "null") == 0)
            {
                type = JSON_NULL;
            }
            else
            {
                type = JSON_INT;
            }
        }
    }
    void json_value::init(const char *data)
    {
        vdata = data;
        detect_type();
        is_loaded = true;
    }
    int str_str_len(const char *data, const char d)
    {
        size_t i;
        for (i = 0; data[i] != d; i++)
        {
        }
        return i;
    }

    char json_data::get_next_data_close_delimitor(int index)
    {
        size_t json_length = higher_storage.size();
        for (size_t i = index; i < json_length; i++)
        {
            if (higher_storage[i] == '}')
            {
                return higher_storage[i];
            }
            if (higher_storage[i] == ',')
            {
                return higher_storage[i];
            }
        }
        return 0;
    }

    json_storage &json_storage::operator[](const char *name)
    {

        for (int i = 0; i < sub_storage.size(); i++)
        {
            if (sub_storage[i]->storage_name != nullptr)
            {
                context.log("%s", sub_storage[i]->storage_name);
                if (strcmp(sub_storage[i]->storage_name, name) == 0)
                {
                    return *sub_storage[i];
                }
            }
        }
        context.generate_error("storage %s not founded", name);
        while (true)
            ;
    }

    json_value json_storage::get_value()
    {
        if (!val.is_valid())
        {
            val.init(storage_value);
        }
        return val;
    }

    void json_data::read()
    {
        json_storage *current_storage = &storage;
        current_storage->parent = nullptr;
        current_storage->storage_name = nullptr;
        current_storage->sub_storage.clear();
        char *current_var_name = "main";
        char *current_var_value;
        size_t json_length = higher_storage.size();
        for (size_t i = 0; i < json_length; i++)
        {
            if (higher_storage[i] == '{')
            {
                json_storage *target = new json_storage;
                target->parent = current_storage;

                target->storage_name = current_var_name;
                target->sub_storage = std::vector<json_storage *>();
                current_storage->sub_storage.push_back(target);
                current_storage = target;
            }
            else if (higher_storage[i] == '"')
            {

                int length = str_str_len(higher_storage.data() + i + 1, '"');
                current_var_name = (char *)malloc(length + 2);
                for (int j = i + 1; higher_storage[j] != '"'; j++)
                {
                    current_var_name[j - i - 1] = higher_storage[j];
                }
                current_var_name[length] = 0;
                i += length + 1;
            }
            else if (higher_storage[i] == ':')
            {
                if (higher_storage[i + 1] == '{')
                {
                    continue;
                }
                char next = get_next_data_close_delimitor(i);
                int length = str_str_len(higher_storage.data() + i + 1, next);
                current_var_value = (char *)malloc(length + 2);

                for (int j = i + 1; higher_storage[j] != next; j++)
                {
                    current_var_value[j - i - 1] = higher_storage[j];
                }
                current_var_value[length] = 0;
                if (next == '}')
                {
                    i += length;
                }
                else
                {
                    i += length + 1;
                }

                json_storage *target = new json_storage;
                target->parent = current_storage;
                target->storage_name = current_var_name;
                target->sub_storage = std::vector<json_storage *>();
                target->storage_value = current_var_value;
                current_storage->sub_storage.push_back(target);
                if (current_storage->storage_name == nullptr)
                {
                    context.log("%s : %s = %s ", "main", current_var_name, current_var_value);
                }
                else
                {
                    context.log("%s : %s = %s ", current_storage->storage_name, current_var_name, current_var_value);
                }
            }
            else if (higher_storage[i] == '}')
            {
                current_storage = current_storage->parent;
            }
        }
    }

    void json_data::create_higher_storage()
    {

        size_t json_length = std::strlen(json_raw_data);
        for (size_t i = 0; i < json_length; i++)
        {
            if (json_raw_data[i] == ' ')
            {
                continue;
            }
            higher_storage.push_back(json_raw_data[i]);
        }
    }
    void json_data::from_file(const char *file_path)
    {
        std::ifstream f(file_path, std::ios::binary | std::ios::ate);
        if (!f.is_open())
        {
            context.generate_error("can't open file %s for json read \n", file_path);
            return;
        }
        std::streamsize size = f.tellg();
        json_raw_data = (char *)malloc((size) + 2);
        f.seekg(0, std::ios::beg);
        f.read(json_raw_data, size);
        json_raw_data[size] = 0;
        create_higher_storage();
        read();
    }
    void json_data::from_data(const char *data)
    {
        json_raw_data = (char *)malloc(strlen(data) + 2);
        strcpy(json_raw_data, data);
        create_higher_storage();
        read();
    }

} // namespace feather