# Copyright 2017 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ros2cli.node.strategy import add_arguments
from ros2cli.node.strategy import NodeStrategy
from ros2node.api import get_node_names
from ros2node.verb import VerbExtension
from mmh3 import hash as mmh3hash

class ListVerb(VerbExtension):
    """Output a list of available nodes."""

    def add_arguments(self, parser, cli_name):
        add_arguments(parser)
        parser.add_argument(
            '-a', '--all', action='store_true',
            help='Display all nodes even hidden ones')
        parser.add_argument(
            '-c', '--count-nodes', action='store_true',
            help='Only display the number of nodes discovered')

    def main(self, *, args):
        with NodeStrategy(args) as node:
            node_names = get_node_names(node=node, include_hidden_nodes=args.all)

        if args.count_nodes:
            print(len(node_names))
        elif node_names:
            print(*[(n.full_name + " (" + self.get_hashed_name(n.full_name) + ")") for n in node_names], sep='\n')

    def get_hashed_name(self, string):
        hash_32_bit = mmh3hash(string); # seed defaults to 0, which we want. But the number could be negative, while we need to treat mmh3 output as uint32_t
        return f"{(hash_32_bit & 0xFFFFFFFF):X}"
