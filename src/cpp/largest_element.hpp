#include <iostream>
using namespace std;
//Iterative Approach - O(n) Time and O(1) Space

//arr[]: mảng cần tìm số lớn nhất
//n : số lượng phần tử trong mảng
//maxvalue : lưu giá trị lớn nhất hiện tại

//Tạo một hàm largest để tìm số lớn nhẩt trong mảng ( int là hàm trả về số nguyên)
//arr trong lúc này là mảng được truyền vào hàm
// n số lượng phần tử của mảng được truyền
int largest(int arr[], int n) {
    //gán phần tử đầu tiên của mảng làm số lớn nhất tạm thời
    int maxValue = arr[0];
    //dùng vòng lặp để chạy thử các trường hợp
    for (int i = 1; i < n; i++) {
        if (arr[i] > maxValue) {
            maxValue = arr[i];
        }
    }

    return maxValue;
}


//Recursive Approach - O(n) Time and O(n) Space
#include <iostream>
using namespace std;

//Tạo hàm findmax để tìm số lớn nhất bằng đệ quy
//arr mảng cần tìm
//i là vị trí hiện tại
//n là số phần tử trong mảng
int findMax(int arr[], int i, int n) {
    //đã tới phần tử cuối của mảng hay chưa
    if (i == n - 1) {
        //tới phần tử cuối thì trả về phần tử đó
        return arr[i];
    }
    //gọi lại hàm findmax
    //lúc này i+1 sẽ là chuyển sang phần tử tiếp theo
    //recMax lưu giá trị lớn nhất tìm được ở phần còn lại của mảng
    int recMax = findMax(arr, i + 1, n);
    //so sánh 
    if (recMax > arr[i]) {
        //nếu recmax lớn hơn phần tử hiện tại là arr[i] thì trả lại về recmax
        return recMax;
    }
    //còn nếu recmax không lớn hơn thì số lớn nhất hiện tại là arr[i]
    else {
        return arr[i];
    }
}
//tạo hàm largest để gọi đệ quy
int largest(int arr[], int n) {
    //Gọi findMax bắt đầu từ vị trí đầu tiên 0.
    return findMax(arr, 0, n);
}

//nfijnsinijdncwnojcncnjq
