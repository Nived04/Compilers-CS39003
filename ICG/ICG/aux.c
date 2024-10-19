long pwr(int base, int exp) {
    long ans = 1;
    while(exp > 0) {
        if(exp%2 == 1) 
            ans = (ans * base);
        base = (base * base);
        exp /= 2;
    }
    return ans;
}

void mprn(int *MEM, int idx) {
    printf("+++ MEM[%d] set to %d\n", idx, MEM[idx]);
}

void eprn(int *Reg, int idx) {
    printf("+++ Standalone expression evaluates to %d\n", Reg[idx]);
}

void parsingError(int line) {
    printf("*** Error found at line %d", line);
}