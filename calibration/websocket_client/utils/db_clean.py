import sqlite3
import os
import argparse

def clean_database(database_file):
    if os.path.exists(database_file):
        try:
            conn = sqlite3.connect(database_file)
            cursor = conn.cursor()
            
            cursor.execute("DELETE FROM sqlite_sequence") 
            cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
            tables = cursor.fetchall()
            for table in tables:
                table_name = table[0]
                cursor.execute(f"DELETE FROM {table_name};")
            
            conn.commit()
            conn.close()
            
            print(f"All data in the database '{database_file}' has been cleared.")
        except sqlite3.Error as e:
            print("SQLite error:", e)
    else:
        print(f"The database file '{database_file}' does not exist.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Clear all data in an SQLite database")
    parser.add_argument("database_file", help="Path to the SQLite database file")

    args = parser.parse_args()
    clean_database(args.database_file)
