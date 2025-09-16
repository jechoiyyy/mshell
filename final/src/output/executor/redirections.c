/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 20:00:39 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 15:36:03 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "executor.h"
#include <stdio.h>

int	input_file_process(t_file *file)
{
	int			fd_in;
	t_filename	*input_file;

	if (!file || !file->input_file)
		return (FAILURE);
	input_file = file->input_file;
	if (!input_file->filename)
		return (FAILURE);
	if (input_file->hd && input_file->hd != -1)
	{
		if (dup2(input_file->hd, STDIN_FILENO) == -1)
		{
			perror("dup2 heredoc");
			return (close(input_file->hd), FAILURE);
		}
		close(input_file->hd);
	}
	else if (input_file->filename && ft_strlen(input_file->filename) > 0)
	{
		if (input_file->flag == 1)
			return (print_error(input_file->filename, "ambiguous redirect"),	\
			FAILURE);
		fd_in = open_input_file(input_file->filename);
		if (fd_in == -1)
			return (FAILURE);
		if (dup2(fd_in, STDIN_FILENO) == -1)
			return (close(fd_in), FAILURE);
		close(fd_in);
	}
	return (SUCCESS);
}

static int	output_file_process(t_file *file)
{
	int			fd_out;
	t_filename	*output_file;

	if (!file || !file->output_file)
		return (FAILURE);
	output_file = file->output_file;
	if (!output_file->filename)
		return (FAILURE);
	if (output_file->flag == 1)
		return (print_error(output_file->filename, "ambiguous redirect"), FAILURE);
	fd_out = open_output_file(output_file->filename, output_file->append_mode);
	if (fd_out == -1)
		return (FAILURE);
	if (dup2(fd_out, STDOUT_FILENO) == -1)
	{
		close(fd_out);
		return (FAILURE);
	}
	close(fd_out);
	return (SUCCESS);
}

int	setup_redirections(t_cmd *cmd)
{
	t_file *current;

	if (!cmd)
		return (FAILURE);
	current = cmd->file;
	while (current)
	{
		if (current->input_file && current->input_file->filename && ft_strlen(current->input_file->filename) > 0)
		{
			if (input_file_process(current) == FAILURE)
				return (FAILURE);
		}
		if (current->output_file && current->output_file->filename && ft_strlen(current->output_file->filename) > 0)
		{
			if (output_file_process(current) == FAILURE)
				return (FAILURE);
		}
		current = current->next;
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
