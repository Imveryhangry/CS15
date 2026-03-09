/*
 * Expense Tracker System - C Implementation
 * Features: Add/edit/delete expenses, categories, budgets, summary
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────────
   Constants
───────────────────────────────────────────── */

#define MAX_EXPENSES   100
#define MAX_BUDGETS    20
#define MAX_STR        100
#define MAX_DATE       11   /* YYYY-MM-DD + null terminator */

/* ─────────────────────────────────────────────
   Structs (equivalent to Python classes)
───────────────────────────────────────────── */

typedef struct {
    int    id;
    float  amount;
    char   category[MAX_STR];
    char   description[MAX_STR];
    char   date[MAX_DATE];
} Expense;

typedef struct {
    char  category[MAX_STR];
    float limit;
} Budget;

typedef struct {
    Expense expenses[MAX_EXPENSES];
    int     expense_count;
    int     next_id;
    Budget  budgets[MAX_BUDGETS];
    int     budget_count;
} ExpenseTracker;

/* ─────────────────────────────────────────────
   Utility Functions
───────────────────────────────────────────── */

/* Convert string to title case e.g. "food" -> "Food" */
void to_title_case(char *str) {
    if (str[0] >= 'a' && str[0] <= 'z')
        str[0] -= 32;
}

/* Remove trailing newline from input */
void strip_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
}

/* Get today's date as YYYY-MM-DD */
void get_today(char *buf) {
    strcpy(buf, "2026-03-01");
}

/* ─────────────────────────────────────────────
   Tracker Initialization
───────────────────────────────────────────── */

void tracker_init(ExpenseTracker *t) {
    t->expense_count = 0;
    t->next_id       = 1;
    t->budget_count  = 0;
}

/* ─────────────────────────────────────────────
   Expense Functions
───────────────────────────────────────────── */

/* Add a new expense */
int add_expense(ExpenseTracker *t, float amount, const char *category,
                const char *description, const char *date) {
    if (t->expense_count >= MAX_EXPENSES) {
        printf("Max expenses reached.\n");
        return -1;
    }

    Expense *e = &t->expenses[t->expense_count];
    e->id     = t->next_id++;
    e->amount = amount;
    strncpy(e->category,    category,    MAX_STR - 1);
    strncpy(e->description, description, MAX_STR - 1);
    strncpy(e->date,        date,        MAX_DATE - 1);
    to_title_case(e->category);

    t->expense_count++;
    return e->id;
}

/* Find expense index by ID, returns -1 if not found */
int find_expense(ExpenseTracker *t, int id) {
    for (int i = 0; i < t->expense_count; i++)
        if (t->expenses[i].id == id)
            return i;
    return -1;
}

/* Edit an existing expense */
int edit_expense(ExpenseTracker *t, int id, float amount, const char *category,
                 const char *description, const char *date) {
    int idx = find_expense(t, id);
    if (idx == -1) return 0;

    Expense *e = &t->expenses[idx];
    if (amount > 0)           e->amount = amount;
    if (strlen(category) > 0) {
        strncpy(e->category, category, MAX_STR - 1);
        to_title_case(e->category);
    }
    if (strlen(description) > 0)
        strncpy(e->description, description, MAX_STR - 1);
    if (strlen(date) > 0)
        strncpy(e->date, date, MAX_DATE - 1);

    return 1;
}

/* Delete an expense by ID */
int delete_expense(ExpenseTracker *t, int id) {
    int idx = find_expense(t, id);
    if (idx == -1) return 0;

    /* Shift all expenses after idx one position to the left */
    for (int i = idx; i < t->expense_count - 1; i++)
        t->expenses[i] = t->expenses[i + 1];

    t->expense_count--;
    return 1;
}

/* Print all expenses, optionally filtered by month */
void list_expenses(ExpenseTracker *t, const char *month, int sort_by_amount, int ascending) {
    Expense *matches[MAX_EXPENSES];
    int count = 0;

    for (int i = 0; i < t->expense_count; i++) {
        if (strlen(month) == 0 || strncmp(t->expenses[i].date, month, 7) == 0)
            matches[count++] = &t->expenses[i];
    }

    if (count == 0) {
        printf("No expenses found.\n");
        return;
    }

    /* Bubble sort */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int swap = 0;
            if (sort_by_amount)
                swap = ascending
                    ? matches[j]->amount > matches[j+1]->amount
                    : matches[j]->amount < matches[j+1]->amount;
            else
                swap = ascending
                    ? strcmp(matches[j]->date, matches[j+1]->date) > 0
                    : strcmp(matches[j]->date, matches[j+1]->date) < 0;
            if (swap) {
                Expense *tmp  = matches[j];
                matches[j]    = matches[j+1];
                matches[j+1]  = tmp;
            }
        }
    }

    printf("\n%-5s %-12s %-15s %8s  %s\n", "ID", "Date", "Category", "Amount", "Description");
    printf("--------------------------------------------------------------\n");

    float total = 0;
    for (int i = 0; i < count; i++) {
        Expense *e = matches[i];
        printf("%-5d %-12s %-15s $%7.2f  %s\n",
               e->id, e->date, e->category, e->amount, e->description);
        total += e->amount;
    }
    printf("\nTotal: $%.2f  (%d entries)\n", total, count);
}

/* ─────────────────────────────────────────────
   Budget Functions
───────────────────────────────────────────── */

/* Find budget index by category, returns -1 if not found */
int find_budget(ExpenseTracker *t, const char *category) {
    for (int i = 0; i < t->budget_count; i++)
        if (strcmp(t->budgets[i].category, category) == 0)
            return i;
    return -1;
}

/* Set or update a budget */
void set_budget(ExpenseTracker *t, const char *category, float limit) {
    int idx = find_budget(t, category);
    if (idx != -1) {
        t->budgets[idx].limit = limit;
        return;
    }
    if (t->budget_count >= MAX_BUDGETS) {
        printf("Max budgets reached.\n");
        return;
    }
    Budget *b = &t->budgets[t->budget_count++];
    strncpy(b->category, category, MAX_STR - 1);
    to_title_case(b->category);
    b->limit = limit;
}

/* Remove a budget */
int remove_budget(ExpenseTracker *t, const char *category) {
    int idx = find_budget(t, category);
    if (idx == -1) return 0;
    for (int i = idx; i < t->budget_count - 1; i++)
        t->budgets[i] = t->budgets[i + 1];
    t->budget_count--;
    return 1;
}

/* ─────────────────────────────────────────────
   Summary Report
───────────────────────────────────────────── */

void summary(ExpenseTracker *t, const char *month) {
    printf("\nPeriod: %s\n", strlen(month) > 0 ? month : "All Time");

    float total = 0;
    int   count = 0;

    char  cats[MAX_BUDGETS][MAX_STR];
    float spent[MAX_BUDGETS];
    int   cat_count = 0;

    for (int i = 0; i < t->expense_count; i++) {
        Expense *e = &t->expenses[i];
        if (strlen(month) > 0 && strncmp(e->date, month, 7) != 0)
            continue;

        total += e->amount;
        count++;

        int found = -1;
        for (int j = 0; j < cat_count; j++)
            if (strcmp(cats[j], e->category) == 0) { found = j; break; }

        if (found == -1) {
            strncpy(cats[cat_count], e->category, MAX_STR - 1);
            spent[cat_count] = e->amount;
            cat_count++;
        } else {
            spent[found] += e->amount;
        }
    }

    printf("Total Spent: $%.2f  (%d transactions)\n\n", total, count);

    if (cat_count == 0) { printf("No expenses recorded.\n"); return; }

    printf("%-18s %9s %9s %10s  Status\n", "Category", "Spent", "Budget", "Remaining");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < cat_count; i++) {
        int bidx = find_budget(t, cats[i]);
        if (bidx != -1) {
            float remaining = t->budgets[bidx].limit - spent[i];
            const char *status = spent[i] > t->budgets[bidx].limit ? "OVER" : "OK";
            printf("%-18s $%8.2f $%8.2f $%9.2f  %s\n",
                   cats[i], spent[i], t->budgets[bidx].limit, remaining, status);
        } else {
            printf("%-18s $%8.2f %9s %10s\n", cats[i], spent[i], "-", "-");
        }
    }
}

/* ─────────────────────────────────────────────
   CLI Menu Actions
───────────────────────────────────────────── */

void menu_add(ExpenseTracker *t) {
    float amount;
    char  category[MAX_STR], description[MAX_STR], date[MAX_DATE + 1];

    printf("\n-- Add Expense --\n");
    printf("Amount ($): ");
    scanf("%f", &amount);
    getchar();

    printf("Category: ");
    fgets(category, MAX_STR, stdin);
    strip_newline(category);

    printf("Description: ");
    fgets(description, MAX_STR, stdin);
    strip_newline(description);

    printf("Date (YYYY-MM-DD) [leave blank for today]: ");
    fgets(date, sizeof(date), stdin);
    strip_newline(date);
    if (strlen(date) == 0) get_today(date);

    int id = add_expense(t, amount, category, description, date);
    if (id != -1)
        printf("Added expense #%d: $%.2f | %s | %s\n", id, amount, category, description);
}

void menu_view(ExpenseTracker *t) {
    char month[8] = "";
    int  sort_by_amount = 0, ascending = 0;
    char buf[4];

    printf("\n-- View Expenses --\n");
    printf("Filter by month YYYY-MM (leave blank for all): ");
    fgets(month, sizeof(month), stdin);
    strip_newline(month);

    printf("Sort by: (1) Date  (2) Amount [default 1]: ");
    fgets(buf, sizeof(buf), stdin);
    if (buf[0] == '2') sort_by_amount = 1;

    printf("Order: (1) Descending  (2) Ascending [default 1]: ");
    fgets(buf, sizeof(buf), stdin);
    if (buf[0] == '2') ascending = 1;

    list_expenses(t, month, sort_by_amount, ascending);
}

void menu_edit(ExpenseTracker *t) {
    int   id;
    float amount = 0;
    char  category[MAX_STR] = "", description[MAX_STR] = "", date[MAX_DATE] = "";
    char  buf[MAX_STR];

    printf("\n-- Edit Expense --\n");
    printf("Expense ID to edit: ");
    scanf("%d", &id);
    getchar();

    int idx = find_expense(t, id);
    if (idx == -1) { printf("Expense not found.\n"); return; }

    Expense *e = &t->expenses[idx];
    printf("Current: #%d | $%.2f | %s | %s | %s\n",
           e->id, e->amount, e->category, e->description, e->date);

    printf("New amount [$%.2f] (0 to skip): ", e->amount);
    scanf("%f", &amount);
    getchar();

    printf("New category [%s] (blank to skip): ", e->category);
    fgets(buf, MAX_STR, stdin); strip_newline(buf);
    if (strlen(buf) > 0) strncpy(category, buf, MAX_STR - 1);

    printf("New description [%s] (blank to skip): ", e->description);
    fgets(buf, MAX_STR, stdin); strip_newline(buf);
    if (strlen(buf) > 0) strncpy(description, buf, MAX_STR - 1);

    printf("New date [%s] (blank to skip): ", e->date);
    fgets(buf, MAX_DATE, stdin); strip_newline(buf);
    if (strlen(buf) > 0) strncpy(date, buf, MAX_DATE - 1);

    edit_expense(t, id, amount, category, description, date);
    printf("Expense #%d updated.\n", id);
}

void menu_delete(ExpenseTracker *t) {
    int  id;
    char confirm[4];

    printf("\n-- Delete Expense --\n");
    printf("Expense ID to delete: ");
    scanf("%d", &id);
    getchar();

    int idx = find_expense(t, id);
    if (idx == -1) { printf("Expense not found.\n"); return; }

    Expense *e = &t->expenses[idx];
    printf("Delete #%d ($%.2f | %s)? (y/n): ", e->id, e->amount, e->description);
    fgets(confirm, sizeof(confirm), stdin);

    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        delete_expense(t, id);
        printf("Expense deleted.\n");
    } else {
        printf("Cancelled.\n");
    }
}

void menu_set_budget(ExpenseTracker *t) {
    char  category[MAX_STR];
    float limit;

    printf("\n-- Set Budget --\n");
    printf("Category: ");
    fgets(category, MAX_STR, stdin);
    strip_newline(category);
    to_title_case(category);

    printf("Monthly budget limit ($): ");
    scanf("%f", &limit);
    getchar();

    set_budget(t, category, limit);
    printf("Budget set: %s -> $%.2f\n", category, limit);
}

void menu_remove_budget(ExpenseTracker *t) {
    char category[MAX_STR];

    printf("\n-- Remove Budget --\n");
    printf("Category: ");
    fgets(category, MAX_STR, stdin);
    strip_newline(category);
    to_title_case(category);

    if (remove_budget(t, category))
        printf("Budget for '%s' removed.\n", category);
    else
        printf("No budget found for '%s'.\n", category);
}

void menu_summary(ExpenseTracker *t) {
    char month[8] = "";
    printf("\n-- Summary --\n");
    printf("Month YYYY-MM (leave blank for all time): ");
    fgets(month, sizeof(month), stdin);
    strip_newline(month);
    summary(t, month);
}

/* ─────────────────────────────────────────────
   Main Menu Loop
───────────────────────────────────────────── */

void print_menu() {
    printf("\n");
    printf("╔══════════════════════════════════╗\n");
    printf("║       EXPENSE TRACKER            ║\n");
    printf("╠══════════════════════════════════╣\n");
    printf("║  1. Add Expense                  ║\n");
    printf("║  2. View Expenses                ║\n");
    printf("║  3. Edit Expense                 ║\n");
    printf("║  4. Delete Expense               ║\n");
    printf("║  5. Set Budget                   ║\n");
    printf("║  6. View Summary / Budget Status ║\n");
    printf("║  7. Remove Budget                ║\n");
    printf("║  0. Exit                         ║\n");
    printf("╚══════════════════════════════════╝\n");
}

int main() {
    ExpenseTracker tracker;
    tracker_init(&tracker);

    printf("\nWelcome to Expense Tracker!\n");

    char choice[4];
    while (1) {
        print_menu();
        printf("Choose an option: ");
        fgets(choice, sizeof(choice), stdin);
        strip_newline(choice);

        switch (choice[0]) {
            case '1': menu_add(&tracker);           break;
            case '2': menu_view(&tracker);          break;
            case '3': menu_edit(&tracker);          break;
            case '4': menu_delete(&tracker);        break;
            case '5': menu_set_budget(&tracker);    break;
            case '6': menu_summary(&tracker);       break;
            case '7': menu_remove_budget(&tracker); break;
            case '0':
                printf("\nGoodbye!\n");
                return 0;
            default:
                printf("Invalid option. Try again.\n");
        }
    }
}