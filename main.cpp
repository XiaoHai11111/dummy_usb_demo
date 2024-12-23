#include <iostream>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <unistd.h> // for usleep

libusb_device_handle* initUSB(uint16_t VID, uint16_t PID);
void handleError(int response);

int main() {
    libusb_device_handle *dev_handle = initUSB(0x1209, 0x0d32);
    if (dev_handle == nullptr) {
        std::cerr << "USB Device Init Failed" << std::endl;
        return 1;
    }

    int actual_length;
    int response;
    std::string input;
    const char *end_sequence = "\r\n";
    char buffer[256];  // 输入字符串
    char read_buffer[512]; // 用于读取数据
    int timeout = 5000; // 设置超时时间

    std::cout << "Enter text to send to the device (type 'exit' to quit): " << std::endl;

    while (true) {
        std::getline(std::cin, input);
        if (input == "exit") {
            break;
        }

        std::string data = input + end_sequence;
        std::strcpy(buffer, data.c_str());

        // 写入数据
        response = libusb_bulk_transfer(dev_handle, 0x01, (unsigned char*)buffer, strlen(buffer), &actual_length, timeout);
        if (response == 0 && actual_length > 0) {
            std::cout << "Send success! " << actual_length << " bytes sent." << std::endl;
        } else {
            std::cerr << "Send failed or timed out." << std::endl;
            handleError(response);
            break;
        }

        // 延时以确保设备有足够时间处理当前指令
        usleep(50000);  // 延时50ms

        // 读取数据
        response = libusb_bulk_transfer(dev_handle, 0x81, (unsigned char*)read_buffer, sizeof(read_buffer), &actual_length, timeout);
        if (response == 0 && actual_length > 0) {
            read_buffer[actual_length] = '\0';  // 确保字符串正确终止
            std::cout << "Received: " << read_buffer << std::endl;
        } else {
            std::cerr << "Error receiving data: " << libusb_strerror((libusb_error)response) << std::endl;
            if (response == 0) {
                std::cerr << "No data received, actual_length = 0." << std::endl;
            }
        }

        // 延时100ms以避免连续请求冲突
        usleep(50000); // 延时50ms
    }

    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(nullptr);
    return 0;
}

libusb_device_handle* initUSB(uint16_t VID, uint16_t PID) {
    libusb_device_handle *dev_handle = nullptr;
    int response = libusb_init(nullptr);
    if (response < 0) {
        std::cerr << "Unable to initialize libusb" << std::endl;
        return nullptr;
    }

    dev_handle = libusb_open_device_with_vid_pid(nullptr, VID, PID);
    if (dev_handle == nullptr) {
        std::cerr << "Unable to open device" << std::endl;
        libusb_exit(nullptr);
        return nullptr;
    }

    if (libusb_kernel_driver_active(dev_handle, 0)) {
        libusb_detach_kernel_driver(dev_handle, 0);
    }

    response = libusb_set_configuration(dev_handle, 1);
    if (response < 0) {
        std::cerr << "Failed to set configuration" << std::endl;
        libusb_close(dev_handle);
        libusb_exit(nullptr);
        return nullptr;
    }

    response = libusb_claim_interface(dev_handle, 0);
    if (response < 0) {
        std::cerr << "Failed to claim interface" << std::endl;
        libusb_close(dev_handle);
        libusb_exit(nullptr);
        return nullptr;
    }

    return dev_handle;
}

void handleError(int response) {
    if (response == LIBUSB_ERROR_TIMEOUT) {
        std::cerr << "Timeout occurred." << std::endl;
    } else if (response == LIBUSB_ERROR_IO) {
        std::cerr << "I/O error occurred." << std::endl;
    } else if (response == LIBUSB_ERROR_NO_DEVICE) {
        std::cerr << "No device found or device was disconnected." << std::endl;
    } else if (response == LIBUSB_ERROR_ACCESS) {
        std::cerr << "Access denied to the USB device." << std::endl;
    } else {
        std::cerr << "Unknown error: " << libusb_strerror((libusb_error)response) << std::endl;
    }
}
