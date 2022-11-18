# OS2


Полная формулировка задачи [здесь](https://se.ifmo.ru/os)
(Лабораторная работа №2)

Необходимо разработать модуль ядра, который бы через интерфейс (в моём случае ioctl) вытаскивал из ядра данные о заданных структурах.

Необходимо вывести данные о структурах multiprocess_signals и page

<details>
<summary>Пример</summary>
Условно, если бы целевой структурой был task_struct, то можно было бы с пользовательского уровня передать модулю pid,  а внутри модуля ядра обратиться к некоторым ее данным. Для простоты, к условному tgid. 
Данные, вытащенные из структуры специально не оговариваются, и могут быть какими угодно. Но должны выводиться в читаемом виде.

```
static int get_task_struct_info(int pid, char *output)
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
}
```

> output - просто любая строка, которую мы потом выведем на пространство пользователя

А затем с помощью программы, исполняемой на пользовательском пространстве, обратиться к модулю ядра и вывести полученные данные.
Желательно, чтобы их достоверность можно было бы проверить иными средствами. 
В случае с tgid, например, с помощью 

``cat /proc/<PID>/status | grep Tgid``
</details>

## Что известно про `multiprocess_signals`

В случае с multiprocess_signals - он встречается в ядре [три](https://github.com/torvalds/linux/search?q=multiprocess_signals) раза

И судя по описанию используется для того, чтобы собирать сигналы которые процесс принимает непосредственно по время выполнения fork()

### [linux/sched/signal.h](https://github.com/torvalds/linux/blob/493ffd6605b2d3d4dc7008ab927dba319f36671f/include/linux/sched/signal.h#L109)

[Здесь объявляется сама структура](https://github.com/torvalds/linux/blob/493ffd6605b2d3d4dc7008ab927dba319f36671f/include/linux/sched/signal.h#L70)

[Поле `multiprocess` в виде связного списка](https://github.com/torvalds/linux/blob/493ffd6605b2d3d4dc7008ab927dba319f36671f/include/linux/sched/signal.h#L109)

### [/kernel/fork.c](https://github.com/torvalds/linux/blob/1440f576022887004f719883acb094e7e0dd4944/kernel/fork.c)

[Здесь](https://github.com/torvalds/linux/blob/1440f576022887004f719883acb094e7e0dd4944/kernel/fork.c#L1996) объявляется поле `delayed` типа `struct multiprocess_signals`

[Здесь](https://github.com/torvalds/linux/blob/1440f576022887004f719883acb094e7e0dd4944/kernel/fork.c#L2065) сигналы принудительно доставляются в `delayed`


[Здесь](https://github.com/torvalds/linux/blob/1440f576022887004f719883acb094e7e0dd4944/kernel/fork.c#L2475) или [здесь](https://github.com/torvalds/linux/blob/1440f576022887004f719883acb094e7e0dd4944/kernel/fork.c#L2554) структура уничтожается

### [linux/signal.c](https://github.com/torvalds/linux/blob/493ffd6605b2d3d4dc7008ab927dba319f36671f/include/linux/sched/signal.h#L109)

[Здесь](https://github.com/torvalds/linux/blob/55be6084c8e0e0ada9278c2ab60b7a584378efda/kernel/signal.c#L1178) структура используется для того, чтобы доставить сигналы процессу после форка

Я использовал макрос `hlist_for_each_entry` для обхода связного списка multiprocess, как в [kernel/signal.c](https://github.com/torvalds/linux/blob/55be6084c8e0e0ada9278c2ab60b7a584378efda/kernel/signal.c)

Проблема в том, что я не могу никак поймать момент, когда бы этот список был не пустым.

## На данный момент

На данный момент налажено взаимодействие между пользовательской программой, запрашивающий данные и модулем ядра. Пока без [хорошей] защиты от дурака.

### Сборка

- `make` - собирает модуль, устанавливает его, создаёт файл-ноду символьного устройства
- `make module` - собирает модуль
- `make install` - устанавливает собранный модуль
- `make create_dev` - создает device файл
- `make remove` - удалять установленный модуль
- `make clean` - удаляет установленный

### Пользовательская программа
При запуске `./user_app` требует два аргумента - `<PID>` процесса, страницы которого вывести, а так же `<PAGE-NUMBER>` - номер страницы процесса, данные о которой необходимо вывести


## Вопросы

Действительно ли я правильно понял цель и смысл структуры multiprocess_signals?

Как остановить пользовательскую программу в специфический момент выполнения fork() после строчек с инициализированной структурой multiprocess_signals?

Если нет, то как она используется и какие условия должны быть соблюдены, чтобы она была не пустая?

Как проверить содержимое этой структуры иными средствами?
