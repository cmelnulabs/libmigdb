# Contributing to libmigdb

Thank you for your interest in contributing to libmigdb! This document outlines the guidelines and standards for contributing to this project.

## Code of Conduct

* Be respectful and constructive in all interactions
* Focus on improving the library for everyone
* Welcome newcomers and help them get started

## How to Contribute

### Reporting Issues

* Use the GitHub issue tracker
* Provide clear description of the problem
* Include steps to reproduce
* Mention your GDB version and platform

### Submitting Changes

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name` or `bug/fix-description`
3. Make your changes following the coding standards below
4. Test your changes thoroughly
5. Commit with clear, descriptive messages (use past participle: "Fixed bug" not "Fix bug")
6. Push to your fork and submit a pull request

## Coding Standards

### General Principles

* **No chunky functions**: Keep functions under 50-60 lines
* **No magic numbers**: Use named constants for all literals (except 0, 1, -1)
* **No uninitialized variables**: Always initialize variables at declaration
* **No ambiguous names**: Avoid single-letter variables (except loop counters), generic names like `tmp`, `data`, `handle()`
* **Maximum nesting**: Keep nesting to 3 levels or less
* **Single Responsibility**: Each function should do one thing well

### Naming Conventions

**Variables:**
```c
int frame_count = 0;           // Good: descriptive
int n = 0;                      // Bad: ambiguous (except in tight loops)

char *result_buffer = NULL;     // Good: clear purpose
char *tmp = NULL;               // Bad: generic

mi_h *gdb_handle = NULL;        // Good: context clear
mi_h *h = NULL;                 // Bad: abbreviated
```

**Functions:**
```c
mi_parse_frame()                // Good: clear action
process_data()                  // Bad: vague verb
handle()                        // Bad: too generic
```

**Constants:**
```c
#define MI_BUFFER_SIZE 1024            // Good
#define MI_MAX_RETRY_COUNT 5           // Good
#define MI_TIMEOUT_SECONDS 10          // Good

// Usage: for (i = 0; i < 10; i++)    // OK: small literal in obvious context
// Usage: buffer[1024]                 // Bad: should use MI_BUFFER_SIZE
```

**Booleans:**
```c
int is_running = 0;             // Good: prefix 'is_'
int has_breakpoint = 0;         // Good: prefix 'has_'
int flag = 0;                   // Bad: unclear meaning
```

### Code Style

**Indentation:** Tabs (width 8) or 4 spaces (be consistent with existing code)

**Braces:**
```c
if (condition) {
    do_something();
}

while (running) {
    process();
}
```

**Error Handling:**
```c
char *buffer = malloc(size);
if (!buffer) {
    return MI_ERROR_MEMORY;
}
```

**Always check:**
* `malloc()`, `calloc()`, `realloc()` return values
* File operations
* String operations that can fail

### Function Structure

**Keep functions focused:**
```c
// Good: single responsibility
static int validate_connection(mi_h *handle) {
    if (!handle) {
        return 0;
    }
    if (!handle->connected) {
        return 0;
    }
    return 1;
}

// Bad: doing too many things
int process_and_validate_and_send(...) {
    // 150 lines of mixed responsibilities
}
```

**Extract helpers for complex logic:**
```c
// Instead of deeply nested conditions:
if (condition1) {
    if (condition2) {
        if (condition3) {
            // deep nesting...
        }
    }
}

// Extract to helper:
static int check_preconditions() {
    return condition1 && condition2 && condition3;
}

if (check_preconditions()) {
    // cleaner logic
}
```

### Memory Management

* **Always free** what you allocate
* **Document ownership** in function comments
* **Use consistent patterns** for allocation/deallocation
* **NULL check** before freeing (though `free(NULL)` is safe)

```c
/**
 * parse_result - Parse MI result string
 * @str: Input string
 *
 * Returns: Allocated result structure or NULL on error.
 *          Caller must free with mi_free_result().
 */
mi_result *parse_result(const char *str);
```

### Comments

**Function documentation:**
```c
/**
 * mi_parse_frame - Parse stack frame information
 * @handle: GDB handle
 * @frame_str: MI frame string
 *
 * Parses a GDB/MI frame response into a structured format.
 *
 * Returns: Allocated mi_frame structure or NULL on error.
 *          Caller is responsible for freeing with mi_free_frame().
 */
```

**Inline comments:**
* Explain *why*, not *what*
* Keep them concise
* Update comments when code changes

```c
// Good: explains reasoning
// Use exponential backoff to avoid overwhelming the connection
sleep_time *= 2;

// Bad: states the obvious
// Multiply sleep_time by 2
sleep_time *= 2;
```

### Testing

* Test your changes on Linux (minimum)
* Verify with multiple GDB versions if possible
* Check for memory leaks with Valgrind
* Ensure examples still compile and run

### Commit Messages

Use past participle (completed action):

**Good:**
```
Fixed buffer overflow in mi_parse_frame

- Added bounds checking for frame buffer
- Replaced magic number with MI_FRAME_BUFFER_SIZE constant
- Added test case for oversized frame data
```

**Bad:**
```
Fix stuff
Update code
Changes
```

**Format:**
```
<type>: <subject> (max 50 chars)

<body explaining what and why, not how>
<wrap at 72 characters>

<optional: references to issues>
Closes #123
```

**Types:** Fixed, Added, Changed, Removed, Improved, Refactored

## Pull Request Process

1. Update documentation if you change behavior
2. Add/update tests if applicable
3. Ensure all tests pass
4. Update README.md if you add features
5. Reference related issues in PR description
6. Wait for review and address feedback

## Questions?

Open an issue with the `question` label or start a discussion.

---

Thank you for contributing to libmigdb!
