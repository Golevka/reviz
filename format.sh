clang-format -style=google -dump-config > .clang-format
git diff -U0 --no-color HEAD^ | python clang-format-diff.py -i -p1