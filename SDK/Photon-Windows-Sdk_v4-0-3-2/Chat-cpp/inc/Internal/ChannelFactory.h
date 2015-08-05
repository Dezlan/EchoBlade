/* Exit Games Photon Chat - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#pragma once

namespace ExitGames
{
	namespace Common
	{
		class JString;
	}

	namespace Chat
	{
		class Client;

		namespace Internal
		{
			class ChannelFactory
			{
				static Channel* create(const Common::JString& name, bool isPrivate);
				static void destroy(const Channel* pChannel);

				friend class Chat::Client;
			};
		}
	}
}