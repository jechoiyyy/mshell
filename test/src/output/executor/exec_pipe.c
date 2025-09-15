/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_pipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 19:57:14 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/15 18:24:03 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "executor.h"
#include "utils.h"

static int	create_pipes(int **pipe_fds, int cmd_count)
{
	int	i;
	int	j;

	if (cmd_count <= 1)
	{
		*pipe_fds = NULL;
		return (SUCCESS);
	}
	*pipe_fds = malloc(sizeof(int) * 2 * (cmd_count - 1));
	if (!*pipe_fds)
		return (FAILURE);
	
	// 모든 FD를 -1로 초기화 (안전을 위해)
	i = 0;
	while (i < (cmd_count - 1) * 2)
	{
		(*pipe_fds)[i] = -1;
		i++;
	}
	
	// 파이프 생성
	i = 0;
	while (i < cmd_count - 1)
	{
		if (pipe(&(*pipe_fds)[i * 2]) == -1)
		{
			perror("pipe");
			// 이미 생성된 파이프들 정리
			j = 0;
			while (j < i)
			{
				if ((*pipe_fds)[j * 2] >= 0)
					close((*pipe_fds)[j * 2]);
				if ((*pipe_fds)[j * 2 + 1] >= 0)
					close((*pipe_fds)[j * 2 + 1]);
				j++;
			}
			free(*pipe_fds);
			*pipe_fds = NULL;
			return (FAILURE);
		}
		i++;
	}
	return (SUCCESS);
}

static int	fork_and_execute(t_cmd *cmd, t_shell *shell, int *pipe_fds, 
							int cmd_index, int cmd_count)
{
	pid_t	pid;

	pid = fork_process(); 
	if (pid == -1)
		return (-1);
	if (pid == 0)
	{
		// 자식 프로세스: 순서 중요!
		// 1. 파이프 설정 및 모든 파이프 FD 닫기
		setup_child_process(cmd, pipe_fds, cmd_index, cmd_count);
		// 2. 리다이렉션 설정 (파이프 설정 후에)
		if (setup_redirections(cmd, cmd_index, cmd_count) == FAILURE)
			exit (FAILURE);
		// 3. 명령어 유효성 검사
		if ((!cmd || !cmd->args || !cmd->args[0]) && cmd->hd == -1)
			exit (FAILURE);
		// 4. 명령어 실행
		if (cmd->args == NULL && cmd->hd > 0)
			g_exit_status = 0;
		else if (is_builtin_command(cmd->args[0]))
			g_exit_status = execute_builtin(cmd, shell);
		else
			g_exit_status = execute_external(cmd, shell);
		exit (g_exit_status);
	}
	else
	{
		// 부모 프로세스
		setup_parent_process(pipe_fds, cmd_index, cmd_count);
	}
	return (pid);
}

static void	cleanup_resources(int *pipe_fds, pid_t *pids, int cmd_count)
{
	if (pipe_fds)
	{
		close_all_pipes(pipe_fds, cmd_count - 1);  // 헤더의 함수 사용
		free(pipe_fds);
	}
	if (pids)
		free(pids);
}

// 의미상: 적용이 필요하면 저장하고 적용한다.
// 성공하면 caller가 나중에 restore 해야 함.
// 실패하면 즉시 되돌려놓고 FAILURE 반환.
static int apply_redirections_in_parent(t_cmd *cmd, int *saved_stdin, int *saved_stdout)
{

    *saved_stdin  = -1;
    *saved_stdout = -1;

	if (cmd->output_file && cmd->output_file->filename && 
		ft_strcmp(cmd->output_file->filename, "NULL") != 0)
	{
		*saved_stdout = dup(STDOUT_FILENO);
	}
	if ((cmd->input_file && cmd->input_file->filename && 
		ft_strcmp(cmd->input_file->filename, "NULL") != 0) ||
		(cmd->hd && cmd->hd != -1))
	{
		*saved_stdin = dup(STDIN_FILENO);
	}
    // 실제 리다이렉션 적용
    if (setup_redirections(cmd, 0, 1) == FAILURE) {
        // 적용 실패 → 저장해둔 것으로 즉시 복구
        if (*saved_stdout != -1)
		{
			dup2(*saved_stdout, STDOUT_FILENO);
			close(*saved_stdout);
			*saved_stdout = -1;
		}
        if (*saved_stdin  != -1) 
		{
			dup2(*saved_stdin,STDIN_FILENO);
			close(*saved_stdin);
			*saved_stdin  = -1;
		}
        return FAILURE;
    }
    return SUCCESS;
}

static void restore_stdio(int *saved_stdin, int *saved_stdout)
{
    // 저장된 값이 있을 때만 복구
    if (*saved_stdout != -1) {
        dup2(*saved_stdout, STDOUT_FILENO);
        close(*saved_stdout);
    }
    if (*saved_stdin != -1) {
        dup2(*saved_stdin, STDIN_FILENO);
        close(*saved_stdin);
    }
}

static int	single_cmd(t_cmd *commands, t_shell *shell)
{
	int saved_stdin;
	int saved_stdout;

	if (apply_redirections_in_parent(commands, &saved_stdin, &saved_stdout) == FAILURE)
        return FAILURE;
	if (commands->args && is_builtin_command(commands->args[0]))
	{
		g_exit_status = execute_builtin(commands, shell);
		restore_stdio(&saved_stdin, &saved_stdout);
		return (g_exit_status);
	}
	if (!commands->args && commands->hd > 0)
	{
    	g_exit_status = 0;
		restore_stdio(&saved_stdin, &saved_stdout);
        return (g_exit_status);
    }
	return (FAILURE);
}

#include "utils.h"

int	execute_pipeline(t_cmd *commands, t_shell *shell)
{
	int		*pipe_fds;
	pid_t	*pids;
	t_cmd	*current;
	int		cmd_count;
	int		i;

	if (!commands || !shell)
		return (FAILURE);
	
	cmd_count = count_commands(commands);

	if (cmd_count == 1 && ((commands->args && is_builtin_command(commands->args[0])) || (commands->hd > 0 && !commands->args)))
    	return (single_cmd(commands, shell));
	// 파이프 생성
	if (create_pipes(&pipe_fds, cmd_count) == FAILURE)
		return (FAILURE);
	
	// PID 배열 할당
	pids = malloc(sizeof(pid_t) * cmd_count);
	if (!pids)
	{
		if (pipe_fds)
		{
			close_all_pipes(pipe_fds, cmd_count - 1);  // 헤더의 함수 사용
			free(pipe_fds);
		}
		return (FAILURE);
	}
	
	// 각 명령어를 포크하여 실행
	current = commands;
	i = 0;
	while (current && i < cmd_count)
	{
		pids[i] = fork_and_execute(current, shell, pipe_fds, i, cmd_count);
		if (pids[i] == -1)
		{
			// 포크 실패 시 이미 생성된 자식 프로세스들을 종료
			while (--i >= 0)
				kill(pids[i], SIGTERM);
			cleanup_resources(pipe_fds, pids, cmd_count);
			return (FAILURE);
		}
		current = current->next;
		i++;
	}
	// 부모 프로세스에서 모든 파이프 닫기
	close_all_pipes(pipe_fds, cmd_count - 1);  // 헤더의 함수 사용
	// 모든 자식 프로세스 종료 대기
	shell->last_exit_status = wait_for_children(pids, cmd_count);
	// 리소스 정리
	cleanup_resources(pipe_fds, pids, cmd_count);
	
	return (SUCCESS);
}
