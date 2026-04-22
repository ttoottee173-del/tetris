# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -Werror -std=c11

# Флаги для покрытия тестами
GCOV_FLAGS = -fprofile-arcs -ftest-coverage

# Библиотеки
LIBS = -lncurses

# Папки
SRC_DIR = .
BRICK_GAME_DIR = $(SRC_DIR)/brick_game/tetris
CLI_DIR = $(SRC_DIR)/gui/cli
TEST_DIR = $(SRC_DIR)/tests

# Исходные файлы
SRC = $(BRICK_GAME_DIR)/tetris.c $(CLI_DIR)/cli.c
OBJ = $(patsubst %.c,%.o,$(SRC))

# Имя исполняемого файла игры
GAME_EXEC = brickgame

# Тестовый файл
TEST_SRC = $(TEST_DIR)/test_tetris.c

# Исполняемый файл для тестов
TEST_EXEC = test_runner

# Библиотека
LIB = libtetris.a
LIB_SRC = $(BRICK_GAME_DIR)/tetris.c
LIB_OBJ = $(BRICK_GAME_DIR)/tetris.o

# Цель по умолчанию
all: $(GAME_EXEC)

# Создание исполняемого файла игры
$(GAME_EXEC): $(LIB) $(CLI_DIR)/cli.o
	$(CC) $(CLI_DIR)/cli.o -L. -ltetris -o $(GAME_EXEC) $(LIBS)

# Компиляция исходных файлов
%.o: %.c
	$(CC) $(CFLAGS) -I$(BRICK_GAME_DIR) -c $< -o $@

# Сборка библиотеки
$(LIB): $(LIB_OBJ)
	ar rcs $(LIB) $(LIB_OBJ)
	ranlib $(LIB)

$(LIB_OBJ): $(LIB_SRC)
	$(CC) $(CFLAGS) -I$(BRICK_GAME_DIR) -c $(LIB_SRC) -o $(LIB_OBJ)

# Сборка и запуск тестов
test: $(GAME_EXEC) $(TEST_SRC)
	@echo "Compiling test executable from: $(TEST_SRC)"
	$(CC) $(CFLAGS) -I$(BRICK_GAME_DIR) $(TEST_SRC) $(LIB_OBJ) -o $(TEST_EXEC) $(LIBS)
	./$(TEST_EXEC)

# Отчёт о покрытии
gcov_report: clean
	$(CC) $(CFLAGS) $(GCOV_FLAGS) -I$(BRICK_GAME_DIR) -c $(BRICK_GAME_DIR)/tetris.c -o $(LIB_OBJ)
	$(CC) $(CFLAGS) $(GCOV_FLAGS) -I$(BRICK_GAME_DIR) $(TEST_SRC) $(LIB_OBJ) -o $(TEST_EXEC) $(LIBS)
	./$(TEST_EXEC)
	gcov $(BRICK_GAME_DIR)/tetris.c
	lcov -t "test" -o test.info -c -d . --rc lcov_branch_coverage=1
	genhtml -o report test.info
	rm -f *.gcno *.gcda *.info *.gcov
	@echo "Coverage report generated in report/index.html"

# Проверка стиля
style:
	clang-format -n --style=Google $(BRICK_GAME_DIR)/*.c $(BRICK_GAME_DIR)/*.h $(CLI_DIR)/*.c $(TEST_DIR)/*.c
	clang-format -i --style=Google $(BRICK_GAME_DIR)/*.c $(BRICK_GAME_DIR)/*.h $(CLI_DIR)/*.c $(TEST_DIR)/*.c

# Проверка утечек памяти
valgrind: test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_EXEC)

# Статический анализ с cppcheck
cppcheck:
	@cppcheck --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --suppress=knownConditionTrueFalse \
		-I ./brick_game/tetris \
		./brick_game/tetris/*.c ./brick_game/tetris/*.h \
		./gui/cli/*.c ./tests/*.c

# Установка
install: $(GAME_EXEC)
	@mkdir -p ../BrickGame/tetris
	@cp $(GAME_EXEC) ../BrickGame/tetris/
	@touch ../BrickGame/tetris/high_score.txt

# Удаление установки
uninstall:
	rm -f ../BrickGame/tetris/$(GAME_EXEC)
	rm -f ../BrickGame/tetris/high_score.txt
	rmdir ../BrickGame/tetris 2>/dev/null || true
	rmdir ../BrickGame 2>/dev/null || true

#Архив
dist: 
	tar -czf ../brickgame.tar.gz $(SRC_DIR)

#Док
dvi: 
	@xdg-open doc/tetris_documentation.txt || echo "Открой src/doc/tetris_documentation.txt вручную"

# Очистка
clean:
	rm -f $(OBJ) $(GAME_EXEC) $(TEST_EXEC) $(LIB_OBJ) $(LIB) *.gcno *.gcda *.gcov *.info *.html *.css high_score.txt
	rm -f $(BRICK_GAME_DIR)/*.gcno $(BRICK_GAME_DIR)/*.gcda
	rm -f ../brickgame.tar.gz
	rm -rf report

# Пересборка
rebuild: clean all

.PHONY: all test gcov_report style valgrind cppcheck install uninstall clean rebuild