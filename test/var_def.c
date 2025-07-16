/* variable redefine */
int a;


int main() {
    int a;
    char a; /* error, redec */
    int b;
    int b;  /* error, redec */
    int c;
    char d;

    {
        char a;
        char b;
        char c;
        int d;
        {
            int a;
            int b;
            int c;
        }
    }
    a = b;
    a = e;	/* error, undec */

    return 0;
}
