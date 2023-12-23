from tabulate import tabulate
import sqlite3
import argparse

column_names = None

def retrieve_rows(database, table, limit):
    global column_names

    connection = sqlite3.connect(database)
    cursor = connection.cursor()
    cursor.execute(f"SELECT * FROM {table} LIMIT {limit}")
    column_names = [description[0] for description in cursor.description]
    rows = cursor.fetchall()
    connection.close()
    return rows

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Retrieve rows from an SQLite database")
    parser.add_argument("database", help="Name of the database file")
    parser.add_argument("table", help="Name of the table in the database")
    parser.add_argument("limit", type=int, help="Number of rows to show")

    args = parser.parse_args()

    database_name = args.database
    table_name = args.table
    limit = args.limit

    rows = retrieve_rows(database_name, table_name, limit)

    if not rows:
        print("No records found in the database.")
    else:
        print(tabulate(rows, headers=column_names, tablefmt="grid"))
