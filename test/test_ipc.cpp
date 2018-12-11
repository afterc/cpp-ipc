#include <thread>
#include <vector>
#include <type_traits>
#include <iostream>

#include "ipc.h"
#include "rw_lock.h"
#include "test.h"

namespace {

class Unit : public TestSuite {
    Q_OBJECT

    const char* name() const {
        return "test_ipc";
    }

private slots:
    void test_send_recv();
    void test_rw_lock();
} unit__;

#include "test_ipc.moc"

void Unit::test_send_recv() {
//    auto h = ipc::connect("my-ipc");
//    QVERIFY(h != nullptr);
//    char data[] = "hello ipc!";
//    QVERIFY(ipc::send(h, data, sizeof(data)));
//    auto got = ipc::recv(h);
//    QCOMPARE((char*)got.data(), data);
//    ipc::disconnect(h);
}

void Unit::test_rw_lock() {
    std::thread r_trd[4];
    std::thread w_trd[4];

    std::vector<int> datas;
    ipc::rw_lock lc;

    for (auto& t : r_trd) {
        t = std::thread([&] {
            std::vector<int> seq;
            std::size_t cnt = 0;
            while (1) {
                int x = -1;
                lc.r_lock();
                if (cnt < datas.size()) {
                    x = datas[cnt];
                }
                lc.r_unlock();
                if (x ==  0) break; // quit
                if (x != -1) {
                    seq.push_back(x);
                    ++cnt;
                }
                std::this_thread::yield();
            }
            std::size_t sum = 0;
            for (int i : seq) {
                sum += static_cast<std::size_t>(i);
            }
            std::cout << std::endl;
            QCOMPARE(sum, 5050 * std::extent<decltype(w_trd)>::value);
        });
    }

    for (auto& t : w_trd) {
        t = std::thread([&] {
            for (int i = 1; i <= 100; ++i) {
                lc.w_lock();
                datas.push_back(i);
                lc.w_unlock();
                std::this_thread::yield();
            }
        });
    }

    for (auto& t : w_trd) t.join();
    lc.w_lock();
    datas.push_back(0);
    lc.w_unlock();

    for (auto& t : r_trd) t.join();

    for (int i : datas) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

} // internal-linkage