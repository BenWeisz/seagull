PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS contacts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    phone_number TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    contact INTEGER NOT NULL,
    is_incoming INTEGER NOT NULL,
    time TEXT NOT NULL,
    body TEXT NOT NULL,
    hash INTEGER NOT NULL,
    FOREIGN KEY (contact) REFERENCES contacts(id)
);