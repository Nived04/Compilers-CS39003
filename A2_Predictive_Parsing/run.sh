lex list.l
g++ parse_list_lang.cpp
./a.out < list.txt
./a.out < biglist.txt

for i in {1..11}; do
    ./a.out < err$i.txt
done
