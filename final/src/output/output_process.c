/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   output_process.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 01:34:42 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 01:34:43 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "output.h"

int	output_process(t_shell *shell, t_prompt *prompt)
{
	t_cmd	*cmd;

	cmd = parse_tokens(prompt);
	if (!cmd)
		return (FAILURE);
	if (execute_pipeline(cmd, shell) == FAILURE)
	{
		free_commands(cmd);
        return (FAILURE);
	}
	free_commands(cmd);
	return (g_exit_status);
}
