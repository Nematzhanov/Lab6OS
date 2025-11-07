//
// Created by Mukhammadzakhid Nematzhanov on 5/11/25.
//
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

using namespace std;

void sem_wait(int semid, int n) {
    struct sembuf op;
    op.sem_num = static_cast<unsigned short>(n);
    op.sem_op = -1;
    op.sem_flg = 0;
    semop(semid, &op, 1);
}

void sem_signal(int semid, int n) {
    struct sembuf op;
    op.sem_num = static_cast<unsigned short>(n);
    op.sem_op = 1;
    op.sem_flg = 0;
    semop(semid, &op, 1);
}


int main() {
    int N, Min, Max;
    cout << "Введите N, Min, Max: ";
    cin >> N >> Min >> Max;

    size_t size = N * sizeof(int);

    int shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    if (shm_id == -1) {
        cerr << "Ошибка shmget";
        return 1;
    }

    int* arr = (int*) shmat(shm_id, nullptr, 0);
    if (arr == (void*) -1) {
        cerr << "Ошибка shmget";
        return 1;
    }

    srand(time(nullptr));
    cout << "Исходный массив:\n";
    for (int i = 0; i < N; ++i) {
        arr[i] = Min + rand() % (Max - Min + 1);
        cout << "  arr[" << i << "] = " << arr[i] << endl;
    }

    int sem_id = semget(IPC_PRIVATE, N, IPC_CREAT | 0666);
    if (sem_id == -1) {
        cerr << "Ошибка shmget";
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        semctl(sem_id, i, SETVAL, 1);
    }

    pid_t pid = fork();

    if (pid < 0) {
       cerr << "Ошибка fork";
        return 1;
    }

    if (pid == 0) {
        cout << "Дочерний процесс начал сортировку\n";

        for (int i = 0; i < N - 1; ++i) {
            for (int j = 0; j < N - i - 1; ++j) {
                sem_wait(sem_id, j);
                sem_wait(sem_id, j + 1);

                if (arr[j] > arr[j + 1]) {
                    int tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }

                sem_signal(sem_id, j);
                sem_signal(sem_id, j + 1);

                usleep(200000);
            }
        }

        cout << "\n Дочерний процесс завершил сортировку.\n";

        shmdt(arr);
        return 0;
    }
    else {
        int iteration = 1;
        int status = 0;

        while (true) {
            cout << "\nИтерация " << iteration++ << "\n";

            for (int i = 0; i < N; ++i) {
                cout << "Родитель: пытается получить доступ к arr[" << i << "]... ";
                sem_wait(sem_id, i);
                cout << "OK, значение = " << arr[i] << endl;
                sem_signal(sem_id, i);
                usleep(150000);
            }

            pid_t res = waitpid(pid, &status, WNOHANG);
            if (res == 0) {
                cout << "Дочерний процесс ещё сортирует...\n";
                sleep(1);
            } else {
                cout << "Дочерний процесс завершился!\n";
                break;
            }
        }

        cout << "\n Отсортированный массив:\n";
        for (int i = 0; i < N; ++i)
            cout << arr[i] << " ";
        cout << endl;

        shmdt(arr);
        shmctl(shm_id, IPC_RMID, nullptr);
        semctl(sem_id, 0, IPC_RMID);
        cout << "Память и семафоры освобождены.\n";
    }

    return 0;
}