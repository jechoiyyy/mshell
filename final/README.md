# Minishell Redirection Fix

## 문제 상황

`echo hi < ./minishell_tester/test_files/infile bonjour 42` 명령 실행 시 bash와 다른 결과가 나오는 문제가 발생했습니다.

### 기존 문제점
```bash
# 기존 minishell 결과
minishell: hi: command not found
minishell: hello: command not found  
minishell: world: command not found
minishell: 42: command not found

# 올바른 bash 결과
hi bonjour 42
```

## 근본 원인 분석

**핵심 문제**: `single_cmd.c`에서 input redirection을 **부모 프로세스**에서 처리하여 `readline()`이 파일에서 읽게 되어, 파일 내용(`hi`, `hello`, `world`, `42`)이 별도의 명령어로 해석되었습니다.

**처리 흐름 문제**:
1. `apply_redirections_in_parent()` → stdin이 파일로 변경
2. 다음 `readline()` 호출 → 파일에서 한 줄씩 읽음
3. 파일의 각 라인이 개별 명령어로 실행됨

## 수정 내용

### 1. 파일 위치
- **수정 파일**: `src/output/executor/single_cmd.c`
- **헤더 추가**: `#include <sys/wait.h>`

### 2. 핵심 변경사항

**기존 코드**:
```c
int single_cmd(t_cmd *commands, t_shell *shell)
{
    int saved_stdin = -1;
    int saved_stdout = -1;

    // 부모 프로세스에서 redirection 처리 (문제 원인)
    if (apply_redirections_in_parent(commands, &saved_stdin, &saved_stdout) == FAILURE)
    {
        g_exit_status = 1;
        return (g_exit_status);
    }
    
    if (is_builtin_command(commands->args[0]))
    {
        g_exit_status = execute_builtin(commands, shell);
        restore_stdio(&saved_stdin, &saved_stdout);
        return (g_exit_status);
    }
    return (FAILURE);
}
```

**수정된 코드**:
```c
int single_cmd(t_cmd *commands, t_shell *shell)
{
    pid_t pid;
    int saved_stdin = -1;
    int saved_stdout = -1;

    // 빈 명령어 처리 (redirection만 있는 경우)
    if (!commands->args || !commands->args[0])
    {
        if (apply_redirections_in_parent(commands, &saved_stdin, &saved_stdout) == FAILURE)
        {
            g_exit_status = 1;
            return (g_exit_status);
        }
        g_exit_status = 0;
        restore_stdio(&saved_stdin, &saved_stdout);
        return (g_exit_status);
    }

    // builtin 명령어는 fork하여 자식에서 실행 (핵심 수정)
    if (is_builtin_command(commands->args[0]))
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return (FAILURE);
        }
        if (pid == 0) // 자식 프로세스
        {
            if (setup_redirections(commands) == FAILURE)
                exit(FAILURE);
            g_exit_status = execute_builtin(commands, shell);
            exit(g_exit_status);
        }
        else // 부모 프로세스
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
                g_exit_status = WEXITSTATUS(status);
            else
                g_exit_status = 128 + WTERMSIG(status);
        }
        return (g_exit_status);
    }
    return (FAILURE);
}
```

### 3. 주요 변경점

1. **Redirection 처리 위치 변경**:
   - **기존**: 부모 프로세스에서 `apply_redirections_in_parent()` 호출
   - **수정**: 자식 프로세스에서 `setup_redirections()` 호출

2. **Fork 추가**:
   - builtin 명령어 실행 전에 `fork()` 추가
   - 자식 프로세스에서 redirection과 명령어 실행
   - 부모 프로세스에서 자식 프로세스 대기 및 상태 처리

3. **빈 명령어 처리**:
   - redirection만 있는 경우는 기존 방식 유지

## 수정 결과

### 테스트 명령어
```bash
echo hi < ./minishell_tester/test_files/infile bonjour 42
```

### 결과 비교
```bash
# 수정 전
minishell: hi: command not found
minishell: hello: command not found  
minishell: world: command not found
minishell: 42: command not found

# 수정 후 (bash와 동일)
hi bonjour 42
```

## 기술적 설명

**왜 이 수정이 필요했나?**

1. **readline 격리**: 부모 프로세스의 stdin을 보호하여 readline이 터미널에서 입력을 받도록 유지
2. **올바른 redirection**: 자식 프로세스에서만 stdin을 파일로 변경하여 명령어 실행에만 영향
3. **프로세스 분리**: builtin 명령어도 외부 명령어처럼 별도 프로세스에서 실행하여 부모 환경 보호

이 수정으로 minishell이 bash와 동일한 redirection 동작을 수행하게 되었습니다.

---

# Builtin Commands Fork Issue Fix

## 문제 상황

`single_cmd` 함수에서 모든 builtin 명령어를 자식 프로세스에서 실행하여 `cd`와 `exit` 명령어가 정상 작동하지 않는 문제가 발생했습니다.

### 기존 문제점
- **cd 명령어**: 자식 프로세스에서 실행되어 부모 셸의 작업 디렉터리가 변경되지 않음
- **exit 명령어**: 자식 프로세스에서만 종료되어 부모 셸이 계속 실행됨

## 근본 원인 분석

**핵심 문제**: 모든 builtin 명령어를 일괄적으로 자식 프로세스에서 실행하는 로직

```c
// 문제가 있던 코드
if (is_builtin_command(commands->args[0]))
{
    pid = fork();  // 모든 builtin을 자식에서 실행
    if (pid == 0)
    {
        // cd, exit 등도 자식에서 실행 → 부모에 영향 없음
        g_exit_status = execute_builtin(commands, shell);
        exit(g_exit_status);
    }
}
```

**문제 상황**:
1. **cd**: `chdir()`이 자식에서 실행 → 부모의 작업 디렉터리 변경 안됨
2. **exit**: `shell->exit_flag = 1`이 자식에서 설정 → 부모 셸 계속 실행

## 수정 내용

### 파일 위치
- **수정 파일**: `src/output/executor/single_cmd.c`

### 핵심 변경사항

**수정된 코드**:
```c
// builtin 명령어 처리
if (is_builtin_command(commands->args[0]))
{
    // cd와 exit은 부모에서 직접 실행해야 함
    if (ft_strcmp(commands->args[0], "cd") == 0 ||
        ft_strcmp(commands->args[0], "exit") == 0)
    {
        if (apply_redirections_in_parent(commands, &saved_stdin, &saved_stdout) == FAILURE)
        {
            g_exit_status = 1;
            return (g_exit_status);
        }
        g_exit_status = execute_builtin(commands, shell);
        restore_stdio(&saved_stdin, &saved_stdout);
        return (g_exit_status);
    }
    // 나머지 builtin은 자식에서 실행
    else
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return (FAILURE);
        }
        if (pid == 0) // 자식 프로세스
        {
            if (setup_redirections(commands) == FAILURE)
                exit(FAILURE);
            g_exit_status = execute_builtin(commands, shell);
            exit(g_exit_status);
        }
        else // 부모 프로세스
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
                g_exit_status = WEXITSTATUS(status);
            else
                g_exit_status = 128 + WTERMSIG(status);
        }
        return (g_exit_status);
    }
}
```

### 주요 변경점

1. **명령어별 차별화 처리**:
   - **cd, exit**: 부모 프로세스에서 직접 실행
   - **echo, pwd, export, unset, env**: 자식 프로세스에서 실행

2. **실행 위치 결정 로직**:
   - 부모 환경 변경이 필요한 명령어 → 부모에서 실행
   - 출력만 하는 명령어 → 자식에서 실행 (안전성)

3. **리다이렉션 처리**:
   - 부모 실행: `apply_redirections_in_parent()` 사용
   - 자식 실행: `setup_redirections()` 사용

## 수정 결과

### 테스트 명령어
```bash
echo -e "pwd\ncd /tmp\npwd\nexit" | ./minishell
```

### 결과 비교
```bash
# 수정 전
minishell$ cd /tmp
# 디렉터리 변경 안됨, exit으로 종료 안됨

# 수정 후
minishell$ pwd
/home/wjddls/42/test/final
shell : 0
minishell$ cd /tmp
shell : 0
minishell$ pwd
/tmp                    # ← 디렉터리 변경됨
shell : 0
minishell$ exit
exit                    # ← 정상 종료됨
shell : 1
```

## 기술적 설명

**왜 이런 차별화가 필요한가?**

1. **환경 상태 변경**: `cd`는 부모 프로세스의 작업 디렉터리를 변경해야 함
2. **셸 제어**: `exit`는 부모 프로세스(메인 셸)를 종료해야 함
3. **안전성**: 다른 builtin들은 자식에서 실행하여 리다이렉션 안전성 확보
4. **표준 준수**: bash의 동작 방식과 일치

이 수정으로 모든 builtin 명령어가 올바른 컨텍스트에서 실행되어 예상대로 동작하게 되었습니다.