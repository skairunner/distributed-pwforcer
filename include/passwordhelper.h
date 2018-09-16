// Includes functions that help with calculating and generating pw ranges

// requires a buffer of length of pwd (64 plus \0)
void calc_pwrange(int nworkers, int rangeindex, short pwlen, unsigned long long* start, unsigned long long* range);
void nth_pwd(unsigned long long index, short pwlen, char* nthpwd);
unsigned long long get_index(char* pwd, short pwdlen);