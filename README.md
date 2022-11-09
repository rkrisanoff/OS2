# OS2


Полная формулировка задачи [здесь](https://se.ifmo.ru/os)
(Лабораторная работа №2)

Необходимо разработать модуль ядра, который бы через интерфейс (в моём случае ioctl) вытаскивал из ядра данные о заданных структурах.
Интерфейс реализован, проблема в другом.

Необходимо вывести данные о структуре multiprocess_signals и page (остальные сделаны)

### Пример
Условно, если бы целевой структурой был task_struct, то можно было бы с пользовательского уровня передать модулю pid,  а внутри модуля ядра обратиться к некоторым ее данным. Для простоты, к условному tgid. 
Данные, вытащенные из структуры специально не оговариваются, и могут быть какими угодно. Но должны выводиться в читаемом виде.



``static int get_task_struct_info(int pid, char *output)
{
    char buff_int[20];
    strcat(output, "get_multiprocess_signals_info -> ");
    sprintf(buff_int, "%d", pid);
    strcat(output, buff_int);
    strcat(output, "\n");

    struct task_struct *task = NULL;
    struct pid *pid_struct = find_get_pid(pid);

    if (!pid_struct)
    {
        return -1;
    }
    task = get_pid_task(pid_struct, PIDTYPE_PID);

    strcat(output, "tgid of current task -> ");
    sprintf(buff_int, "%d", task->tgid);
    strcat(output, buff_int);
    strcat(output, "\n");
}``

> output - просто любая строка, которую мы потом выведем на пространство пользователя

А затем с помощью программы, исполняемой на пользовательском пространстве, обратиться к модулю ядра и вывести полученные данные.
Желательно, чтобы их достоверность можно было бы проверить иными средствами. 
В случае с tgid, например, с помощью 

``cat /proc/<PID>/status | grep Tgid``

### Что известно про `multiprocess_signals`
В случае с multiprocess_signals - он встречается в ядре [три](https://github.com/torvalds/linux/search?q=multiprocess_signals) раза
![image](https://user-images.githubusercontent.com/87571811/200832786-e57cbc96-b571-49f9-9246-081d56b75a23.png)

И судя по описанию используется для того, [чтобы собирать сигналы](https://github.com/torvalds/linux/blob/493ffd6605b2d3d4dc7008ab927dba319f36671f/include/linux/sched/signal.h),
которые процесс принимает непосредственно по время выполнения [fork()](https://github.com/torvalds/linux/blob/1440f576022887004f719883acb094e7e0dd4944/kernel/fork.c)

Здесь объявляется

![image](https://user-images.githubusercontent.com/87571811/200835468-308c5786-1d8a-4503-bade-2ecb770debfd.png)

Здесь используется

![image](https://user-images.githubusercontent.com/87571811/200835721-eea9ba5d-266b-4d3f-aadd-6cb8d7ef5447.png)

См комментарии

![image](https://user-images.githubusercontent.com/87571811/200835787-62b2e949-8cb7-4896-be07-616879174a11.png)



Я использовал макрос для обхода связного списка multiprocess, как в [kernel/signal.c](https://github.com/torvalds/linux/blob/55be6084c8e0e0ada9278c2ab60b7a584378efda/kernel/signal.c)
![image](https://user-images.githubusercontent.com/87571811/200835872-3c7e21fc-f440-4b03-a321-f17c2eed8425.png)


Проблема в том, что я не могу никак поймать момент, когда бы этот список был не пустым.


# На данный момент

На данный момент налажено взаимодействие между пользовательской программой, запрашивающий данные и модулем ядра. Пока без защиты от дурака.
Все запросы выглядят так (на них можно проверить работоспособность):

Выводит список сетевых устройств

> net_device

Выводит состояние регистров на момент обращения

> pt_regs

Должен выводить multiprocess_signals

> ms \<PID\>

Выводит данные о первой странице памяти, которую процесс использует

> pg \<PID\>

Обработка их осуществляется в [здесь](https://github.com/rkrisanoff/OS2/blob/cc247247df277e70c502477e4373ad114eb4a685/drukhary_km.c#L289)

`make` - собирает модуль, устанавливает его, создаёт файл-ноду символьного устройства и запускает пользовательскую программу
`make clean` - удаляет модуль

