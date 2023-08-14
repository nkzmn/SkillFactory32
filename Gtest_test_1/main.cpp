#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>

class DBconnectionInterface
{
public:
    DBconnectionInterface() {}
    virtual ~DBconnectionInterface() {}
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool execQuery(const std::string& str) = 0;
private:
};

class DBconnection : public DBconnectionInterface
{
public:
    DBconnection()
    {
        std::cout << "constructor " << this << "\n";
        descriptor = nullptr;
    }

    virtual ~DBconnection()
    {
        std::cout << "destructor " << this << "\n";
        delete descriptor;
    }

    virtual bool open() override
    {
        if ((descriptor == nullptr) || (*descriptor < 0))
        {
            srand(111);
            descriptor = new int((rand() % 10000) + 1);
            return true;
        }
        else return false;
    }

    virtual void close() override
    {
        if (descriptor == nullptr) return;
        *descriptor = -1;
    }

    virtual bool execQuery(const std::string& str) override
    {
        if (descriptor == nullptr) return false;
        else if (*descriptor < 0) return false;
        else
        {
            std::cout << str << "\n";
            return true;
        }
    }

private:
    int* descriptor;
};


class ClassThatUsesDB
{
public:
    ClassThatUsesDB(DBconnectionInterface* connection)
    {
        m_conn = connection;
    }

    bool openConnection()
    {
        if (m_conn->open()) 
        {
            std::cout << "OPENED\n";
            return true;
        }
        else
        {
            std::cout << "NOT OPENED\n";
            return false;
        }
    }

    bool useConnection(const std::string& str)
    {
        if (m_conn->execQuery(str))
        {
            std::cout << "EXEC\n";
            return true;
        }
        else
        {
            std::cout << "NOT EXEC\n";
            return false;
        }
    }

    void closeConnection()
    {
        m_conn->close();
    }

private:
    DBconnectionInterface* m_conn;
};

class MockDBconnection : public DBconnectionInterface
{
public:
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, execQuery, (const std::string& str), (override));
};

class SomeTestSuite : public ::testing::Test
{
protected:
    void SetUp()
    {
        dbci = new DBconnection();
        dbconn = new ClassThatUsesDB(dbci);
    }

    void TearDown()
    {
        delete dbconn;
        delete dbci;
    }

protected:
    DBconnectionInterface* dbci;
    ClassThatUsesDB* dbconn;
};


TEST_F(SomeTestSuite, testcase1)        //тест котрытия соединения
{
    bool test = dbconn->openConnection();
    bool reference(true);
    ASSERT_EQ(test, reference);
}

TEST_F(SomeTestSuite, testcase2)        //тест, который показывает, что если соединение не открыто, то делать запрос нельзя
{
    bool test = dbconn->useConnection("123");
    bool reference(false);
    ASSERT_EQ(test, reference);
}

TEST_F(SomeTestSuite, testcase3)        //тестирование запросов
{
    dbconn->openConnection();
    bool test = dbconn->useConnection("123");
    bool reference(true);
    ASSERT_EQ(test, reference);
}

TEST_F(SomeTestSuite, testcase4)        //тестирование повторного открытия
{
    dbconn->openConnection();
    bool test = dbconn->openConnection();
    bool reference(false);
    ASSERT_EQ(test, reference);
}

TEST_F(SomeTestSuite, testcase5)        //тест на обмен с использованием мок-объектов
{

    //создаем мок-объект
    MockDBconnection mdbc;
    
    //прописываем «ожидания»
    EXPECT_CALL(mdbc, open).WillOnce(::testing::Return(true));
    //EXPECT_CALL(mdbc, open).WillOnce(::testing::Return(false));

    EXPECT_CALL(mdbc, execQuery("123")).WillOnce(::testing::Return(true));


    //запускаем алгоритм на обработку
    ClassThatUsesDB CTUDB(&mdbc);
    bool result1 = CTUDB.openConnection();
    bool result2 = CTUDB.useConnection("123");
    

    //сравниваем полученный результат с референсом
    ASSERT_EQ(true, result1);
    ASSERT_EQ(true, result2);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}