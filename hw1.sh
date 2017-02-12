# Q2:

# (1)
# ./reviz '1((00)|(01)|(10)|(11))*'   # starts with 1 and has an odd length
# ./reviz '0((00)|(01)|(10)|(11))*(0|1)' # starts with 0 and has even length
./redot '( 1((00)|(01)|(10)|(11))* ) | ( 0( (00)|(01)|(10)|(11) )*(0|1))'
dot -Tpng dfa_opt.dot  -o q2.1.png
dot -Tps dfa_opt.dot -o q2.1.ps
mv q2.1.* ./sample

# ./reviz '(1((00)|(01)|(10)|(11))*)|(0((00)|(01)|(10)|(11))*(0|1))'
#!!! wrong!!! pay attention to parenthesis

# (2)
# not in a*b*, so cant be empty,either starting from a or b
# start from a:
# ./reviz 'a(a|b)*ba(a|b)*'

# start from b:
# ./reviz 'b(a|b)*a(a|b)*'

# or
./redot '(a|b)*ba(a|b)*'
dot -Tpng dfa_opt.dot  -o q2.2.png
dot -Tps dfa_opt.dot -o q2.2.ps
mv q2.2.* ./sample

# (3)
./redot '(0|(10)|(110)|(1110)|(11110))(0|1)*'
dot -Tpng dfa_opt.dot  -o q2.3.png
dot -Tps dfa_opt.dot -o q2.3.ps
mv q2.3.* ./sample

# (4)
./redot 'b(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)*(d|m)'
dot -Tpng dfa_opt.dot  -o q2.4.png
dot -Tps dfa_opt.dot -o q2.4.ps
mv q2.4.* ./sample

# (5)
./redot '1*2*3*4*5*6*7*8*9*'
dot -Tpng dfa_opt.dot  -o q2.5.png
dot -Tps dfa_opt.dot -o q2.5.ps
mv q2.5.* ./sample

# Q3
# ./reviz '(0|1)*'
# ./reviz '(0011)|(0101)|(0110)|(1001)|(1010)|(1100)'

./redot '(0|1)*((0011)|(0101)|(0110)|(1001)|(1010)|(1100))'
dot -Tpng dfa_opt.dot  -o q3.png
dot -Tps dfa_opt.dot -o q3.ps
mv q3.* ./sample

# Q4

# (1)
# {Ø|a|aa|aaa|...|a^k},where a^k is concatenation of k a’s
# ./reviz 'a|(aa)|(aaa)'

# (2)
# ./reviz 'a(Ø|(a)|(aa)|(aaa))(Ø|b|bb|bbb|bbbb)b'

# (3)
./redot '(aaaaaa)(aaaaaa)*'
dot -Tpng dfa_opt.dot  -o q3.3.png
dot -Tps dfa_opt.dot -o q3.3.ps
mv q3.3.* ./sample

# (4)
# not regular

# ./reviz '0*1(0|10*1)*'
# http://www.cs.odu.edu/~toida/nerzic/390teched/regular/reg-lang/examples.html
# Simply put, it is the set of strings 
# over the alphabet { a, b }
#  that contain an odd number of b's 
# ./reviz '0*1(0*10*1)*0*'


# ./reviz '(?<!foo|bar)(foo|bar)(?!foo|bar)'
# ./reviz '(?<!foo|bar)(foo|bar)(?!foo|bar)'

# ./reviz '0((0|1)(0|1))*|1(0|1)((0|1)(0|1))*'

# ./reviz '((a|b|c)*|(d|e)+)f'
# ./reviz '(a|b)*abb'
# ./reviz '(0|1)*(0|1)(0011)|(0101)|(0110)|(1001)|(1010)|(1100)'
# book example for minimized DFA using Brozowski, Page 76 of Cooper 2nd
# ./reviz '(abc)|(bc)|(ad)'


# dot -Tps dfa_opt.dot -o dfa_opt.ps
# dot -Tps dfa.dot -o dfa.ps
# dot -Tps nfa.dot -o nfa.ps
# evince dfa_opt.ps
# evince *.ps
rm *.ps
rm *.dot

