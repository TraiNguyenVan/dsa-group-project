#include <iostream>
using namespace std;

// hash function
int hashFunction(int key, int size) { return key % size; }

/*
-------------------------------PHẦN NÀY TRONG TEST 1------------------------
Một hash function tốt cần có 3 yếu tố là : NHANH, INDEX HỢP LỆ, PHÂN BỐ TƯƠNG ĐỐI ĐỀU
Nếu Hash xấu → nhiều collision → Hash Table chậm.
Hash Function không đảm bảo mỗi key có một index khác nhau.
Trong hash nếu có 3 key bị trùng nhau lúc này collision sẽ xuất hiện
Khi tới BẢNG BĂM chúng ta có công thức của hash function là h(k)=k % 10
ta có int table[10]; cái này gọi là hash table có nghĩa là table [0] tới table[9] đều được gọi là
hash table khi ta chèn một phần tử X nào đó thì lúc này sẽ được tính là x % 10 = y thì cái y nó
sẽ là table[y] của thằng x
Tiếp theo chúng ta tới phần search thì search ở trong hash nó sẽ nhanh
hơn rất nhiều chúng ta ko cần duyệt mảng từ đầu tới cuối mà chúng ta có thể lất số đó chia lấy dư
với 10 thì chúng ta sẽ tìm được vị trí của nó luôn và đó gọi là hash table nhanh
Và Insert với lại search sẽ sở hưu chung một cấu trúc : key -> hash -> index -> table[index]
Nhưng khi này hash table vẫn chưa hoàn chỉnh có nghĩa là nếu 2 dữ liệu trùng trong một table thì
nó sẽ ghi đè trực tiếp lên mà ko chèn vào => lúc này chúng ta có collision Vậy collision là x !=
y nhưng h(x) = h(y) có nghĩa là chúng ta có 2 giá trị x và y khác nhau nhưng nó sẽ lưu chung vào
một index và khôngbị ghi đè lên nhau

-------------------------------PHẦN NÀY LÀ TEST 2----------------------------------------
Khi này chúng ta sẽ đến với SEPARATE CHAINING
Ví dụ nếu chung ta có 2 số x và y chung một table thì lúc này chúng ta sẽ cho table trỏ tới một danh
sách có cả x và y khi này link list sẽ xuất hiện Lúc này chúng ta cần sẽ thay đổi cấu trúc của nó là
chúng ta thêm tạo thêm một phần struct Node lúc này chúng ta sẽ hình dung là cái bucket của chúng ta
sẽ được mở rộng lên không phải bị giới hạn bởi chỉ một phần tử trong đó mà chúng ta có thể chèn thêm
2 3 thậm chí là n phần tử vào nó


*/
