#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <dirent.h>
#include <ctime>
#include <algorithm>
using std::vector;
using std::sort;
using std::string;
using std::cout;
using std::endl;
using std::make_pair;
using std::getline;
using std::cin;
using std::pair;
using std::distance;
using std::ifstream;
using std::unordered_map;
using std::map;

vector<string> get_file_paths(string path = "./training/"){
    vector<string> file_dir;
    file_dir.reserve(8);
    DIR *dir;
    dirent *pdir;
    dir = opendir(path.c_str());

    cout<<"Attempt to load training directory..."<<endl;
    if(!dir){
        cout<<"Training directory cannot be opened"<<endl;
        exit(1);
    }
    cout<<"Traning directory successfully loaded!"<<endl<<endl;
    while((pdir = readdir(dir))){
        if(pdir->d_name[0] != '.'){
            file_dir.push_back(pdir->d_name);
        }
    }
    return file_dir;
}


unordered_map<string, map<char, int>> get_counts(const vector<string> &file_dir){
    char c;
    string lang = "";
    ifstream in_file;
    unordered_map<string, map<char, int>> counts;
    for(const string &dir : file_dir){
        in_file.open("./training/"+dir);
        if(!in_file){
            cout<<"Unable to read file "<<dir<<endl;
            continue;
        }
        lang = dir.substr(0, dir.find('.'));
        //insert into counts map
        if(counts.find(lang) == counts.end()){
            map<char, int> tmp;
            counts.emplace(make_pair(lang, tmp));
        }
        else exit(1);
        //read the file char by char
        while(in_file.get(c)){
            if(isalpha(c)){
                char tmp = tolower(c);
                //check if already inserted, if not, insert
                if(counts.at(lang).find(tmp) == counts.at(lang).end()){
                    counts.at(lang).emplace(make_pair(tmp, 1));
                }
                else{
                    (counts.at(lang).at(tmp))++;
                }
            } 
        }
        in_file.close();
    }
    return counts;
}

void print_freq(const unordered_map<string, map<char, int>> &counts){
    for(const auto &outter : counts){
        cout<<outter.first<<": ";
        map<int, char> freq;
        for(const auto &inner : outter.second){
            freq.emplace(make_pair(inner.second, inner.first));
        }

        for(auto itr = freq.rbegin(); itr != freq.rend(); ++itr){
            cout<<itr->second;
        }
        cout<<endl;
    }
    cout<<endl;
}

void generate_pred(const vector<char> &freq,  
const unordered_map<string, map<char, int>> &counts, int threshold){
    unordered_map<string, int> pred;
    int diff = 0;
    for(const auto &outter : counts){
        pred.emplace(make_pair(outter.first, 0));
        map<int, char> tmp;
        vector<char> lang;
        //populate a map in reversed key value order
        for(const auto &inner : outter.second){
            tmp.emplace(make_pair(inner.second, inner.first));
        }
        //insert into a indexed based container
        for(auto itr = tmp.rbegin(); itr != tmp.rend(); ++itr){
            lang.emplace_back(itr->second);
        }
        //iterate through user input
        for(int i = 0; i < lang.size(); ++i){
            //compute positional difference
            auto itr = find(freq.begin(), freq.end(), lang[i]);
            int tmp_dist = abs(i - distance(freq.begin(), itr));
            if(tmp_dist >= threshold){
                diff += tmp_dist;
            }
        }
        pred.at(outter.first) += diff;
        diff = 0;
    }
    cout<<"Diffs: ";
    vector<pair<string, int>> diffs;
    for(const auto &itr : pred){
        diffs.emplace_back(make_pair(itr.first, itr.second));
    }

    sort(diffs.begin(), diffs.end(), [&](const auto &i, const auto &j){
        return i.second < j.second;
    });

    for(const auto &itr : diffs){
        cout<<"("<<itr.first<<": "<<itr.second<<")"<<" ";
    }
    cout<<endl<<endl<<"Language: "<<diffs[0].first<<endl;
    cout<<endl;
}

vector<char> generate_input_freq(const string &user_input){
    map<char, int> input;
    string alphabet = "abcdefghijklmnopqrstuvwxyz";
    for(const char &c : alphabet){
        input.emplace(make_pair(c, 0));
    }
    for(const char &c : user_input){
        if(isalpha(c)){
            char tmp = tolower(c);
            input.at(tmp)++;
        }
    }
    cout<<"Input: ";
    vector<char> freq;
    vector<pair<int, char>> letters;
    for(const auto &itr : input){
        letters.emplace_back(make_pair(itr.second, itr.first));
        cout<<itr.second<<" ";
    }
    cout<<endl<<"Input: ";
    sort(letters.begin(), letters.end(), [&](const auto &i, const auto &j){
        return i.first > j.first;
    });
    for(const auto &itr : letters){
        cout<<itr.second;
        freq.emplace_back(itr.second);
    }
    cout<<endl<<endl;
    return freq;
}

vector<char> default_input(){
    ifstream in_file;
    string input = "", tmp;
    in_file.open("./input.txt");
    if(!in_file){
        cout<<"Unable to read file ./input.txt"<<endl;
        exit(1);
    }
    while(in_file >> tmp){
        input += tmp;
    }
    in_file.close();
    vector<char> input_freq = generate_input_freq(input);
    return input_freq;
}



int main(){
    clock_t begin = clock();    
    cout<<"** Training... **"<<endl<<endl;
    //get all the file paths
    vector<string> file_dir = get_file_paths();
    //process all the data files and generate letter frequency
    unordered_map<string, map<char, int>> counts = get_counts(file_dir);
    for(const auto &outter : counts){
        cout<<outter.first<<": ";
        for(const auto &inner : outter.second){
            cout<<inner.second<<" ";
        }
        cout<<endl;
    }
    
    //rank letter by frequency grouped by language
    cout<<endl<<"** Letter Frequency Order (High -> Low) **"<<endl;
    print_freq(counts);

    string input;
    string text = "";
    cout<<"Please enter text, following by # (default = 'input.txt')"<<endl;
    bool flag = false;
    vector<char> def_input;
    while(getline(cin, input)){
        if(input.empty()){
            flag = true;
            def_input = default_input();
            break;
        }
        else if(input.compare("#") == 0){
            break;
        }
        text += input;
    }
    string threshold;

    cout<<"Enter difference threshold (default = 4) > ";
    getline(cin, threshold);
    if(threshold.empty()){
        threshold = "4";
    }

    if(!flag){
        vector<char> user_freq = generate_input_freq(text);
        generate_pred(user_freq, counts, stoi(threshold));
    }
    else{
        generate_pred(def_input, counts, stoi(threshold));
    }
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout<<"Execution Time : "<<elapsed_secs<<endl;
}