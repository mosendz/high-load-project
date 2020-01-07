# Web-сервис, предоставляющий счёт текущих матчей
## Суть сервиса
Данный сервис предоставляет текущий счёт футбольных матчей.

## API
API веб-сервиса будет предоставляться посредством HTTP, а также реализовывать принципы REST.

### Предстваления
Представления будут иметь формат JSON

#### `match`
Содержит названия команд `team1` и `team2`, а также счёт, хранящийся по ключам `score1` и `score2`. Опциональный ключ `self` использутся в ответе на запрос для хранения URI ресурса по которому возможно его получение или изменение с помощью соответствующих HTTP методов `GET` и `PUT`.

    {
        "team1": "Real Madrid",
        "team2": "Barcelona",
        "score1": 3,
        "score2": 2,
        "self": "/matches/42"
    }
    
#### Коллекция `matchlist`
По ключу `matches` содержит список объектов `match`. По ключу `self` хранится URI коллекции.

    {
        "matches": [match],
        "self": "/matches"
    }
    
### Виды запросов

#### Получение стартового списка сервисов
HTTP метод: `GET /`
Тип ответа: 

#### Получить список матчей
HTTP метод: `GET`
Тип ответа: `matches` 

#### Получение информации об отдельном матче
Тип HTTP запроса: `GET`
Тип ответа: `match`

#### Добавить матч
Тип HTTP запроса: `POST`
Тип запроса: `match`
Тип ответа: `match`

#### Изменение матча
Тип HTTP запроса: `PUT`
Тип запроса: `match`
Тип ответа: `match`
    
## Соблюдение ограничений REST

### Модель клиент-сервер
С помощью модели клиент-сервер будет произведено разграничение обязоностей клиента и сервера.

### Отсутствие состояния
Сервер не хранит состояние клиента и каждый пришедший на сервер запрос обрабатывается вне контекста пришедших с того же клиента запросов.

### Кэширование
Ответы на запросы на получение информации от матчах можно хранить в кэше до изменения запрашиваемых матчей.

### Единообразие интерфейса

#### Индентификация ресурсов
Представления в формате JSON не привязаны к внутреннему формату данных на сервере. Каждый ресурс имеет постоянный URI, который не зависит от изменения данных ресурса.

#### Манипуляция ресрсами через представление
Клиент отправляет на сервер измененённые или добавлямые ресурсы в виде представления. За сервером остаётся полный контроль над процессом управления русурсами.

#### Самоописываемые сообщения
Сам по себе формат JSON не предоставляет возможности описания данных, а следовательно сообщения, отправляемые в данном формате не будут самоописывающимися. Однако данное требование будет выполнятся, если предположить, что указанные выше представления являются зарегистрированными vendor-specific MIME-типами (например, application/vnd.livematch.match+json).

#### Гипермедиа как средство изменения состояния приложения (HATEOAS)
Солгласно данному ограничению переход получение информации о доступных ресурсах производится посредством перехода по гиперссылкам, начиная с заглавного ресурса, предосталяющего набор доступных возможностей.

## Реализация
В ходе работы над проектом были реализованы три вида запроса: получение списка матчей, получение информации об отдельном матче и изменение матча.
