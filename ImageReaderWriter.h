#define BGVALUE 0

bool readImage(char* imagepath, float*** featurevectors, int* width, int* height);
bool readImageandLabels(char* imagepath, char* labelpath, float** labels, float*** featurevectors, int* width, int* height, int* numannotations);
bool writeLabels(float* labels, int width, int height, char* outputfilename);