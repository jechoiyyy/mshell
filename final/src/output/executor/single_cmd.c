/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   single_cmd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 13:48:35 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 16:12:48 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "executor.h"
#include "utils.h"
#include <sys/wait.h>


// static int check_file_requirements(t_cmd *cmd)
// {
//     t_file *current;
//     char    *dir;
    
//     if (!cmd || !cmd->file)
//         return SUCCESS;
    
//     current = cmd->file;
//     while (current)
//     {
//         if (current->input_file && current->input_file->filename)
//         {
//             if (current->input_file->hd != -1)
//             {
//                 if (current->input_file->hd < 0)
//                     return (FAILURE);
//             }
//             else
//             {
//                 if (access(current->input_file->filename, F_OK) != 0)
//                 {
//                     print_error(current->input_file->filename, strerror(errno));
//                     return FAILURE;
//                 }
//                 if (access(current->input_file->filename, R_OK) != 0)
//                 {
//                     print_error(current->input_file->filename, strerror(errno));
//                     return FAILURE;
//                 }
//             }
//         }
//         if (current->output_file && current->output_file->filename)
//         {
//             if (access(current->output_file->filename, F_OK) == 0)
//             {
//                 if (access(current->output_file->filename, W_OK) != 0)
//                 {
// 		            print_error(current->output_file->filename, strerror(errno));
//                     return (FAILURE);
//                 }
//             }
//             else
//             {
//                 dir = ft_dirname(current->output_file->filename);
//                 if (!dir)
//                     return (FAILURE);
//                 if (access(dir, F_OK) != 0)
//                 {
// 		            print_error(dir, strerror(errno));
//                     free(dir);
//                     return (FAILURE);
//                 }
//                 if (access(dir, W_OK) != 0)
//                 {
// 		            print_error(dir, strerror(errno));
//                     free(dir);
//                     return (FAILURE);
//                 }
//                 free(dir);
//             }
//         }
//         current = current->next;
//     }
//     return (SUCCESS);
// }

static int has_redirections(t_cmd *cmd)
{
    t_file *current;
    
    if (!cmd || !cmd->file)
        return (FAILURE);
    
    current = cmd->file;
    while (current)
    {
        if ((current->input_file && current->input_file->filename && 
             ft_strcmp(current->input_file->filename, "NULL") != 0) ||
            (current->output_file && current->output_file->filename && 
             ft_strcmp(current->output_file->filename, "NULL") != 0))
        {
            return (SUCCESS);
        }
        current = current->next;
    }
    return (FAILURE);
}

static int apply_redirections_in_parent(t_cmd *cmd, int *saved_stdin, int *saved_stdout)
{

    *saved_stdin  = -1;
    *saved_stdout = -1;


    // if (check_file_requirements(cmd) == FAILURE)
    //     return (FAILURE);
    if (has_redirections(cmd))
    {
        *saved_stdin = dup(STDIN_FILENO);
        *saved_stdout = dup(STDOUT_FILENO);
        
        if (*saved_stdin == -1 || *saved_stdout == -1)
        {
            if (*saved_stdin != -1)
                close(*saved_stdin);
            if (*saved_stdout != -1)
                close(*saved_stdout);
            return (FAILURE);
        }
    }
    if (setup_redirections(cmd) == FAILURE)
	{
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
        return (FAILURE);
    }
    return (SUCCESS);
}

static void restore_stdio(int *saved_stdin, int *saved_stdout)
{
    if (*saved_stdout != -1) {
        dup2(*saved_stdout, STDOUT_FILENO);
        close(*saved_stdout);
    }
    if (*saved_stdin != -1) {
        dup2(*saved_stdin, STDIN_FILENO);
        close(*saved_stdin);
    }
}

int	single_cmd(t_cmd *commands, t_shell *shell)
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

    // builtin 명령어는 fork하여 자식에서 실행
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
