
from datetime import datetime
from collections import defaultdict


# ─────────────────────────────────────────────
#  Data Classes
# ─────────────────────────────────────────────

class Expense:
    # initialize with amount, category, description, date (optional), and expense ID

    def __init__(self, amount: float, category: str, description: str,
                 date: str = None, expense_id: int = None):
        self.expense_id = expense_id
        self.amount = round(float(amount), 2)
        self.category = category.strip().title()
        self.description = description.strip()
        self.date = date or datetime.now().strftime("%Y-%m-%d")
    # represent a human readable string of the expense for debugging and display purposes
    def __repr__(self):
        return (f"Expense(id={self.expense_id}, ${self.amount:.2f}, "
                f"'{self.category}', '{self.description}', {self.date})")


class Budget:
    # initialize with category and limit amount
    def __init__(self, category: str, limit: float):
        self.category = category.strip().title()
        self.limit = round(float(limit), 2)
    # represent a human readable string of the budget for debugging and display purposes
    def __repr__(self):
        return f"Budget('{self.category}', limit=${self.limit:.2f})"


#  Core Manager

class ExpenseTracker:
    # initialize with empty expenses list, budgets dict, and next ID counter
    def __init__(self):
        self._expenses: list[Expense] = []
        self._budgets: dict[str, Budget] = {}
        self._next_id: int = 1

    # Expenses CRUD

    def add_expense(self, amount: float, category: str,
                    description: str, date: str = None) -> Expense:
        expense = Expense(amount, category, description, date, self._next_id)
        self._next_id += 1
        self._expenses.append(expense)
        return expense

    def get_expense(self, expense_id: int) -> Expense | None:
        for e in self._expenses:
            if e.expense_id == expense_id:
                return e
        return None
# kwargs define every possible parameter upfront
# kwargs can use any of the following keys: amount, category, description, date
    def edit_expense(self, expense_id: int, **kwargs) -> bool:
        expense = self.get_expense(expense_id)
        if not expense:
            return False
        for key, val in kwargs.items():
            if key == "amount":
                expense.amount = round(float(val), 2)
            elif key == "category":
                expense.category = str(val).strip().title()
            elif key == "description":
                expense.description = str(val).strip()
            elif key == "date":
                expense.date = val
        return True

    def delete_expense(self, expense_id: int) -> bool:
        before = len(self._expenses)
        self._expenses = [e for e in self._expenses if e.expense_id != expense_id]
        return len(self._expenses) < before

    def list_expenses(self, category: str = None, month: str = None,
                      sort_by: str = "date", ascending: bool = False) -> list[Expense]:
        result = self._expenses
        if category:
            result = [e for e in result if e.category == category.strip().title()]
        if month:
            result = [e for e in result if e.date.startswith(month)]
        key = (lambda e: e.amount) if sort_by == "amount" else (lambda e: e.date)
        return sorted(result, key=key, reverse=not ascending)

    # ── Budgets ───────────────────────────────

    def set_budget(self, category: str, limit: float) -> Budget:
        budget = Budget(category, limit)
        self._budgets[budget.category] = budget
        return budget

    def get_budget(self, category: str) -> Budget | None:
        return self._budgets.get(category.strip().title())

    def remove_budget(self, category: str) -> bool:
        key = category.strip().title()
        if key in self._budgets:
            del self._budgets[key]
            return True
        return False

    # ── Reports ───────────────────────────────

    def summary(self, month: str = None) -> dict:
        """Return spending summary, optionally filtered by month."""
        expenses = self.list_expenses(month=month)
        total = sum(e.amount for e in expenses)
        by_category = defaultdict(float)
        for e in expenses:
            by_category[e.category] += e.amount

        budget_status = {}
        for cat, spent in by_category.items():
            budget = self._budgets.get(cat)
            budget_status[cat] = {
                "spent": round(spent, 2),
                "limit": budget.limit if budget else None,
                "remaining": round(budget.limit - spent, 2) if budget else None,
                "over_budget": spent > budget.limit if budget else False,
            }

        return {
            "period": month or "All Time",
            "total": round(total, 2),
            "count": len(expenses),
            "by_category": dict(budget_status),
        }

    @property
    def categories(self) -> list[str]:
        return sorted({e.category for e in self._expenses})



# Interface


class ExpenseTrackerInterface:

    MENU = """
╔══════════════════════════════════╗
║       EXPENSE TRACKER            ║
╠══════════════════════════════════╣
║  1. Add Expense                  ║
║  2. View Expenses                ║
║  3. Edit Expense                 ║
║  4. Delete Expense               ║
║  5. Set Budget                   ║
║  6. View Summary / Budget Status ║
║  7. Remove Budget                ║
║  0. Exit                         ║
╚══════════════════════════════════╝"""

    def __init__(self):
        self.tracker = ExpenseTracker()

    def run(self):
        print("\nWelcome to Expense Tracker!")
        while True:
            print(self.MENU)
            choice = input("Choose an option: ").strip()
            actions = {
                "1": self._add_expense,
                "2": self._view_expenses,
                "3": self._edit_expense,
                "4": self._delete_expense,
                "5": self._set_budget,
                "6": self._view_summary,
                "7": self._remove_budget,
                "0": self._exit,
            }
            action = actions.get(choice)
            if action:
                action()
            else:
                print("❌ Invalid option. Try again.")

    # ── Menu Actions ──────────────────────────

    def _add_expense(self):
        print("\n── Add Expense ──")
        try:
            amount = float(input("Amount ($): "))
            category = input("Category (e.g. Food, Transport): ")
            description = input("Description: ")
            date_str = input("Date (YYYY-MM-DD) [leave blank for today]: ").strip()
            date = date_str if date_str else None
            expense = self.tracker.add_expense(amount, category, description, date)
            print(f"✅ Added: {expense}")
        except ValueError:
            print("❌ Invalid amount.")

    def _view_expenses(self):
        print("\n── View Expenses ──")
        cat = input("Filter by category (leave blank for all): ").strip()
        month = input("Filter by month YYYY-MM (leave blank for all): ").strip()
        print("Sort by: (1) Date  (2) Amount  [default: 1]")
        sort_choice = input("Sort option: ").strip()
        sort_by = "amount" if sort_choice == "2" else "date"
        print("Order: (1) Descending  (2) Ascending  [default: 1]")
        order_choice = input("Order option: ").strip()
        ascending = order_choice == "2"
        expenses = self.tracker.list_expenses(
            category=cat or None, month=month or None,
            sort_by=sort_by, ascending=ascending)
        if not expenses:
            print("No expenses found.")
            return
        print(f"\n{'ID':<5} {'Date':<12} {'Category':<15} {'Amount':>8}  Description")
        print("─" * 60)
        for e in expenses:
            print(f"{e.expense_id:<5} {e.date:<12} {e.category:<15} ${e.amount:>7.2f}  {e.description}")
        print(f"\nTotal: ${sum(e.amount for e in expenses):.2f}  ({len(expenses)} entries)")

    def _edit_expense(self):
        print("\n── Edit Expense ──")
        try:
            eid = int(input("Expense ID to edit: "))
            expense = self.tracker.get_expense(eid)
            if not expense:
                print("❌ Expense not found.")
                return
            print(f"Current: {expense}")
            updates = {}
            val = input(f"New amount [${expense.amount}]: ").strip()
            if val: updates["amount"] = float(val)
            val = input(f"New category [{expense.category}]: ").strip()
            if val: updates["category"] = val
            val = input(f"New description [{expense.description}]: ").strip()
            if val: updates["description"] = val
            val = input(f"New date [{expense.date}]: ").strip()
            if val: updates["date"] = val
            if updates:
                self.tracker.edit_expense(eid, **updates)
                print(f"✅ Updated: {self.tracker.get_expense(eid)}")
            else:
                print("No changes made.")
        except ValueError:
            print("❌ Invalid input.")

    def _delete_expense(self):
        print("\n── Delete Expense ──")
        try:
            eid = int(input("Expense ID to delete: "))
            expense = self.tracker.get_expense(eid)
            if not expense:
                print("❌ Expense not found.")
                return
            confirm = input(f"Delete '{expense}'? (y/n): ").strip().lower()
            if confirm == "y":
                self.tracker.delete_expense(eid)
                print("✅ Expense deleted.")
        except ValueError:
            print("❌ Invalid ID.")

    def _set_budget(self):
        print("\n── Set Budget ──")
        try:
            category = input("Category: ")
            limit = float(input("Monthly budget limit ($): "))
            budget = self.tracker.set_budget(category, limit)
            print(f"✅ Budget set: {budget}")
        except ValueError:
            print("❌ Invalid amount.")

    def _remove_budget(self):
        print("\n── Remove Budget ──")
        category = input("Category: ")
        if self.tracker.remove_budget(category):
            print(f"✅ Budget for '{category.title()}' removed.")
        else:
            print("❌ No budget found for that category.")

    def _view_summary(self):
        print("\n── Summary ──")
        month = input("Month YYYY-MM (leave blank for all time): ").strip()
        summary = self.tracker.summary(month=month or None)
        print(f"\n📊 Period: {summary['period']}")
        print(f"   Total Spent: ${summary['total']:.2f}  ({summary['count']} transactions)\n")
        if summary["by_category"]:
            print(f"{'Category':<18} {'Spent':>9} {'Budget':>9} {'Remaining':>10}  Status")
            print("─" * 60)
            for cat, data in sorted(summary["by_category"].items()):
                limit = f"${data['limit']:.2f}" if data["limit"] else "  —"
                remaining = f"${data['remaining']:.2f}" if data["remaining"] is not None else "  —"
                status = "🔴 OVER" if data["over_budget"] else ("🟢 OK" if data["limit"] else "")
                print(f"{cat:<18} ${data['spent']:>8.2f} {limit:>9} {remaining:>10}  {status}")
        else:
            print("No expenses recorded.")

    def _exit(self):
        print("\nGoodbye!")
        raise SystemExit  

if __name__ == "__main__":
    ExpenseTrackerInterface().run()