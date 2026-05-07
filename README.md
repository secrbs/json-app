# json-app

JSON 파일을 영속성 저장소로 사용하는 **태스크 관리 CRUD CLI** 애플리케이션입니다.  
외부 라이브러리 없이 C++17 표준만으로 구현되었으며, JSON I/O는 [secrbs/json-poc](https://github.com/secrbs/json-poc) 라이브러리를 참조합니다.

## 요구사항

| 항목 | 버전 |
|------|------|
| Visual Studio | 2022 (MSVC v143) |
| C++ 표준 | C++17 |
| 의존 프로젝트 | [json-poc](https://github.com/secrbs/json-poc) — 동일 부모 디렉토리에 클론 필요 |

### 디렉토리 구조

json-poc와 json-app은 **같은 부모 디렉토리** 아래에 있어야 합니다.

```
parent/
├── json_poc/      ← https://github.com/secrbs/json-poc
│   └── json_poc/
│       ├── json.h
│       └── json.cpp
└── json_app/      ← 이 저장소
    └── json_app/
        ├── task.h
        ├── store.h / store.cpp
        └── main.cpp
```

json_app 프로젝트는 json_poc의 소스를 상대 경로(`..\..\json_poc\json_poc`)로 직접 참조하므로 별도 빌드·설치가 필요 없습니다.

## 빌드

```
git clone https://github.com/secrbs/json-app
cd json-app
MSBuild json_app.sln /p:Configuration=Debug /p:Platform=x64
```

실행 파일 생성 위치: `x64\Debug\json_app.exe`

## 사용법

```
json_app <command> [args]
```

### 커맨드 목록

| 커맨드 | 인수 | 설명 |
|--------|------|------|
| `add`  | `<title> [--tag TAG]` | 태스크 추가 |
| `list` | `[--done\|--pending] [--tag TAG]` | 태스크 목록 |
| `show` | `<id>` | 태스크 상세 보기 |
| `done` | `<id>` | 완료 처리 |
| `undo` | `<id>` | 미완료로 되돌리기 |
| `edit` | `<id> [--title T] [--tag TAG]` | 수정 |
| `rm`   | `<id>` | 삭제 |
| `help` | | 도움말 출력 |

데이터는 **실행 디렉토리의 `tasks.json`** 에 자동 저장됩니다.

---

## 예제

### 태스크 추가

```
> json_app add "Buy groceries" --tag shopping
Added: #1  Buy groceries

> json_app add "Finish the report" --tag work
Added: #2  Finish the report

> json_app add "Read Clean Code" --tag study
Added: #3  Read Clean Code
```

### 목록 조회

```
> json_app list
ID    DONE   TAG           TITLE
----  -----  ------------  ----------------------------------------
1     [ ]    shopping      Buy groceries
2     [ ]    work          Finish the report
3     [ ]    study         Read Clean Code
  3 task(s)
```

### 필터링

```
> json_app list --tag work
ID    DONE   TAG           TITLE
----  -----  ------------  ----------------------------------------
2     [ ]    work          Finish the report
  1 task(s)

> json_app list --pending
ID    DONE   TAG           TITLE
----  -----  ------------  ----------------------------------------
1     [ ]    shopping      Buy groceries
3     [ ]    study         Read Clean Code
  2 task(s)
```

### 상세 보기

```
> json_app show 1
ID      : 1
Title   : Buy groceries
Tag     : shopping
Done    : no
Created : 2026-05-07
```

### 완료 처리 / 되돌리기

```
> json_app done 2
Task #2 marked as done

> json_app list --done
ID    DONE   TAG           TITLE
----  -----  ------------  ----------------------------------------
2     [x]    work          Finish the report
  1 task(s)

> json_app undo 2
Task #2 marked as pending
```

### 수정

```
> json_app edit 3 --title "Read The Pragmatic Programmer" --tag study
Updated task #3

> json_app edit 1 --tag errands
Updated task #1
```

### 삭제

```
> json_app rm 2
Deleted task #2

> json_app list
ID    DONE   TAG           TITLE
----  -----  ------------  ----------------------------------------
1     [ ]    errands       Buy groceries
3     [ ]    study         Read The Pragmatic Programmer
  2 task(s)
```

---

## 데이터 파일

태스크는 현재 디렉토리의 `tasks.json`에 저장됩니다.

```json
{
  "next_id": 4,
  "tasks": [
    {
      "created_at": "2026-05-07",
      "done": false,
      "id": 1,
      "tag": "errands",
      "title": "Buy groceries"
    },
    {
      "created_at": "2026-05-07",
      "done": false,
      "id": 3,
      "tag": "study",
      "title": "Read The Pragmatic Programmer"
    }
  ]
}
```

| 필드 | 타입 | 설명 |
|------|------|------|
| `next_id` | number | 다음에 발급할 ID (자동 증가) |
| `tasks[].id` | number | 태스크 고유 ID |
| `tasks[].title` | string | 제목 |
| `tasks[].tag` | string | 태그 (선택) |
| `tasks[].done` | bool | 완료 여부 |
| `tasks[].created_at` | string | 생성일 (`YYYY-MM-DD`) |

---

## 코드 구조

```
json_app/
├── task.h       Task 데이터 구조체
├── store.h      Store 클래스 선언
├── store.cpp    CRUD 구현 및 JSON 파일 I/O
└── main.cpp     CLI 파싱 및 커맨드 디스패치
```

### Store 클래스

```cpp
class Store {
public:
    explicit Store(const std::string& path);  // tasks.json 경로

    Task              add(const std::string& title, const std::string& tag = "");
    std::vector<Task> list() const;
    Task              get(int id) const;
    void              mark_done(int id, bool done);
    void              update(int id, const std::string* title, const std::string* tag);
    void              remove(int id);
};
```

- 생성자에서 파일을 로드하고, 파일이 없으면 빈 데이터로 초기화합니다.
- 데이터를 변경하는 모든 메서드는 변경 후 즉시 파일에 저장합니다.
- `update`의 `title` / `tag` 인수는 포인터로 전달하며, `nullptr`이면 해당 필드를 수정하지 않습니다.
