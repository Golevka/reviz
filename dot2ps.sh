valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./redot '( 1((00)|(01)|(10)|(11))* ) | ( 0( (00)|(01)|(10)|(11) )*(0|1))'


dot -Tps dfa_opt.dot -o dfa_opt.ps
dot -Tps dfa.dot -o dfa.ps
dot -Tps nfa.dot -o nfa.ps
evince *.ps

rm *.dot
rm *.ps