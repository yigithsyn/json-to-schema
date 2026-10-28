CC ?= cc
CFLAGS ?= -O2
CPPFLAGS ?= -Iinclude
PYTHON ?= python

SOURCES = src/jsmn.c src/json_schema.c
TEST_BINARY = build/test_json_schema
CLI_BINARY = build/json-to-schema

.PHONY: all cli test clean docs docs-serve

all: cli test

$(CLI_BINARY): $(SOURCES) src/json_to_schema_cli.c include/jsmn.h include/json_schema.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=c89 -pedantic -Wall -Wextra -Werror \
		$(SOURCES) src/json_to_schema_cli.c -o $(CLI_BINARY)

cli: $(CLI_BINARY)

$(TEST_BINARY): $(SOURCES) tests/test_json_schema.c include/jsmn.h include/json_schema.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=c89 -pedantic -Wall -Wextra -Werror \
		$(SOURCES) tests/test_json_schema.c -o $(TEST_BINARY)

test: $(TEST_BINARY) $(CLI_BINARY)
	./$(TEST_BINARY)
	./$(CLI_BINARY) -h >/dev/null
	./$(CLI_BINARY) --help >/dev/null
	./$(CLI_BINARY) --version >/dev/null

docs:
	$(PYTHON) -m mkdocs build --strict

docs-serve:
	$(PYTHON) -m mkdocs serve

clean:
	rm -rf build
