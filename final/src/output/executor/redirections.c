/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 20:00:39 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 01:35:44 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "executor.h"
#include <stdio.h>

static int	input_file_process(t_cmd *cmd)
{
	int			fd_in;
	t_filename	*file;

	file = cmd->input_file;
	while (file)
	{
		if (file->hd && file->hd != -1)
		{
			if (dup2(file->hd, STDIN_FILENO) == -1)
			{
				perror("dup2 heredoc");
				return (close(file->hd), FAILURE);
			}
			close(file->hd);
		}
		else if (file->filename && ft_strlen(file->filename) > 0)
		{
			if (file->flag == 1)
				return (print_error(file->filename, "ambiguous redirect"),	\
				FAILURE);
			fd_in = open_input_file(file->filename);
			if (fd_in == -1)
				return (FAILURE);
			if (dup2(fd_in, STDIN_FILENO) == -1)
				return (close(fd_in), FAILURE);
			close(fd_in);
		}
		file = file->next;
	}
	return (SUCCESS);
}

int	setup_redirections(t_cmd *cmd, int cmd_index, int cmd_count)
{
	int			fd_out;
	t_filename *file;

	(void)cmd_index;
	(void)cmd_count;
	if (!cmd)
		return (FAILURE);
	fd_out = -1;
	if (input_file_process(cmd) == FAILURE)
		return (FAILURE);
	if (cmd->output_file && cmd->output_file->filename && ft_strlen(cmd->output_file->filename) > 0)
	{
		file = cmd->output_file;
		while (file)
		{
			if (file->flag == 1)
				return (print_error(file->filename, "ambiguous redirect"), FAILURE);
			fd_out = open_output_file(file->filename, file->append_mode);
			if (fd_out == -1)
				return (FAILURE);
			if (dup2(fd_out, STDOUT_FILENO) == -1)
			{
				close(fd_out);
				return (FAILURE);
			}
			close(fd_out);
			file = file->next;
		}
	}
	return (SUCCESS);
}

int	open_input_file(char *filename)
{
	int	fd;

	if (!filename)
		return (-1);
	fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		print_error(filename, strerror(errno));
		return (-1);
	}
	return (fd);
}

int	open_output_file(char *filename, int append_mode)
{
	int	fd;
	int	flags;

	if (!filename)
		return (-1);
	flags = O_WRONLY | O_CREAT;
	if (append_mode)
		flags |= O_APPEND;
	else
		flags |= O_TRUNC;
	fd = open(filename, flags, 0644);
	if (fd == -1)
	{
		print_error(filename, strerror(errno));
		return (-1);
	}
	return (fd);
}
