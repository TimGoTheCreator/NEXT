// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "struct/particle.h"
#include "floatdef.h"
#include <fstream>
#include <string>
#include <vector>


std::vector<Particle> LoadParticlesFromFile(const std::string& filename)
{
    std::vector<Particle> p;
    std::ifstream in(filename);

    Particle temp;
    // Match this to the file columns: x y z vx vy vz m type
    while (in >> temp.x >> temp.y >> temp.z >> temp.vx >> temp.vy >> temp.vz >> temp.m >> temp.type) {
        p.push_back(temp);
    }

    return p;
}
