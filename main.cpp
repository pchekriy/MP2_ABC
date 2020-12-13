#include <iostream>
#include <thread>
#include <mutex>
#include <utility>
#include <vector>
#include <sstream>
#include <chrono>

using namespace std;
string names[10] = {"Oliver", "Jack", "Harry", "Jacob", "Tomas",
                    "Amelia", "Olivia", "Emily", "Jessica", "Sophie"};
string surnames[10] = {"Adams", "Becker", "Livingston", "Carroll", "Milton",
                       "Derrick", "Palmer", "Young", "Taylor", "White"};

class Book {
private:
    string name;
    string text;
    string last_author;

    string generate_name();

    string generate_text();

    const string alphabet_lower = "abcdefghijklmnopqrstuvwxyz";
    const string alphabet_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    static int random_num_letter();

public:
    explicit Book(string &last_author);

    explicit Book(string &last_author, string &name);

    explicit Book(string &last_author, string &name, string &text);

    string get_name();

    string get_last_author();

    string get_text();

    void change_text();

    void change_author(string &author);
};

vector<Book> bd;
mutex w, r, reader_m, author_m;
int count_r, count_w;

int choose_random_book() {
    cout << "There are " << bd.size() << " books in the library" << endl;
    return rand() % bd.size();
}

class Reader {
protected:
    string name;
public:
    explicit Reader();

    virtual void read();

    virtual void func_thread();
};

class Author : Reader {
public:
    explicit Author();

    void read() override;

    void write();

    void func_thread() override;
};

int Book::random_num_letter() {
    return rand() % 26;
}

string Book::generate_name() {
    stringstream ans;
    ans << alphabet_upper[random_num_letter()];
    int len = rand() % 5 + 3;
    while (len--)
        ans << alphabet_lower[random_num_letter()];
    return ans.str();
}

string Book::generate_text() {
    stringstream ans;
    int len = rand() % 5 + 3;
    while (len--)
        ans << generate_name() << " ";
    return ans.str();
}

Book::Book(string &last_author) {
    this->last_author = last_author;
    name = generate_name();
    text = generate_text();
}

Book::Book(string &last_author, string &name) {
    this->last_author = last_author;
    this->name = name;
    text = generate_text();
}

Book::Book(string &last_author, string &name, string &text) {
    this->last_author = last_author;
    this->name = name;
    this->text = text;
}

string Book::get_name() {
    return name;
}

string Book::get_last_author() {
    return last_author;
}

string Book::get_text() {
    return text;
}

void Book::change_text() {
    text = generate_text();
}

void Book::change_author(string &author) {
    last_author = author;
}

Reader::Reader() {
    name = names[rand() % 10] + " " + surnames[rand() % 10];
}

void Reader::read() {
    if (bd.empty()) {
        cout << "No books in the library yet" << endl;
        cout << name << " can not read anyone" << endl;
        return;
    }
    r.lock();
    reader_m.lock();
    if (++count_r == 1)
        w.lock();
    reader_m.unlock();
    r.unlock();

    reader_m.lock();
    int book = choose_random_book();
    cout << name << " want read the book " << bd[book].get_name() << ", " << bd[book].get_last_author() << endl;
    reader_m.unlock();
    chrono::duration<int> sleep_time(rand() % 3 + 1);
    this_thread::sleep_for(sleep_time);
    reader_m.lock();
    cout << name << " finished reading the book" << endl;
    cout << "Text: " << bd[book].get_text() << endl << endl;
    reader_m.unlock();

    reader_m.lock();
    if (--count_r == 0)
        w.unlock();
    reader_m.unlock();
}

void Reader::func_thread() {
    cout << name << " register in the library as reader" << endl;
    for (int i = 0; i < 4; ++i) {
        this_thread::sleep_for(chrono::duration<int>(rand() % 3 + 2));
        read();
    }
    cout << "Goodbye! " << name << " leave from the library" << endl << endl;
}

Author::Author() : Reader() {}

void Author::read() {
    if (bd.empty()) {
        cout << "No books in the library yet" << endl;
        cout << name << " can not read anyone" << endl;
        return;
    }
    author_m.lock();
    if (++count_w == 1)
        r.lock();
    author_m.unlock();

    w.lock();
    int book = choose_random_book();
    cout << name << " want read the book " << bd[book].get_name() << ", " << bd[book].get_last_author() << endl;
    chrono::duration<int> sleep_time(rand() % 3 + 1);
    this_thread::sleep_for(sleep_time);
    cout << name << " finished reading the book" << endl;
    cout << "Text: " << bd[book].get_text() << endl << endl;
    w.unlock();

    author_m.lock();
    if (--count_w == 0)
        r.unlock();
    author_m.unlock();
}

void Author::write() {
    author_m.lock();
    if (++count_w == 1)
        r.lock();
    author_m.unlock();

    w.lock();
    if (bd.empty() || rand() % 2) {
        Book book(name);
        cout << name << " want write new book" << endl;
        chrono::duration<int> sleep_time(rand() % 3 + 1);
        this_thread::sleep_for(sleep_time);
        bd.push_back(book);
        cout << name << " finished write the book" << endl << endl;
    } else {
        int book = choose_random_book();
        cout << name << " want rewrite the book " << bd[book].get_name()
             << ", " << bd[book].get_last_author() << endl;
        chrono::duration<int> sleep_time(rand() % 3 + 1);
        this_thread::sleep_for(sleep_time);
        bd[book].change_text();
        bd[book].change_author(name);
        cout << name << " finished rewrite the book" << endl << endl;
    }
    w.unlock();

    author_m.lock();
    if (--count_w == 0)
        r.unlock();
    author_m.unlock();
}

void Author::func_thread() {
    cout << name << " register in the library as author" << endl;
    write();
    for (int i = 0; i < 7; ++i) {
        this_thread::sleep_for(chrono::duration<int>(rand() % 3 + 2));
        if (rand() % 2)
            read();
        else
            write();
    }
    cout << "Goodbye! " << name << " leave from the library" << endl << endl;
}


int main() {
    vector<Reader> readers;
    vector<Author> authors;

    for (int i = 0; i < 3; ++i) {
        readers.push_back(Reader());
        authors.push_back(Author());
    }

    vector<thread> threads(6);

    for (int i = 0; i < 3; ++i)
        threads[i] = thread(&Reader::func_thread, readers[i]);

    for (int i = 3; i < 6; ++i)
        threads[i] = thread(&Author::func_thread, authors[i-3]);

    for (int i = 0; i < 6; ++i)
        threads[i].join();


    return 0;
}