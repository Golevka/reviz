clang-format -style=Google -dump-config > .clang-format
git diff -U0 --no-color HEAD^ | python clang-format-diff.py -i -p1

# valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./redot '( 1((00)|(01)|(10)|(11))* ) | ( 0( (00)|(01)|(10)|(11) )*(0|1))'
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./redot '(0|1)*((0011)|(0101)|(0110)|(1001)|(1010)|(1100))'