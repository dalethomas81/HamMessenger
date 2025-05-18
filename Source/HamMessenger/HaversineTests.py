import math
import unittest

EARTH_RADIUS_MI = 3958.8  # Miles

def aprs_to_decimal(coord_str, hemisphere):
    """
    Convert APRS uncompressed position (e.g., '2415.364N') to decimal degrees.
    """
    if len(coord_str) < 4:
        raise ValueError("Coordinate string too short")

    if hemisphere in ['N', 'S']:
        degrees = int(coord_str[:2])
        minutes = float(coord_str[2:])
    elif hemisphere in ['E', 'W']:
        degrees = int(coord_str[:3])
        minutes = float(coord_str[3:])
    else:
        raise ValueError("Invalid hemisphere")

    decimal = degrees + (minutes / 60.0)
    if hemisphere in ['S', 'W']:
        decimal = -decimal

    return decimal

def haversine_miles(lat1, lon1, lat2, lon2):
    to_rad = math.pi / 180.0
    dlat = (lat2 - lat1) * to_rad
    dlon = (lon2 - lon1) * to_rad

    a = math.sin(dlat / 2)**2 + math.cos(lat1 * to_rad) * math.cos(lat2 * to_rad) * math.sin(dlon / 2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

    return EARTH_RADIUS_MI * c

class TestAPRSPosition(unittest.TestCase):
    def test_sf_to_la(self):
        # San Francisco: 3746.494N, 12225.164W
        # Los Angeles:   3403.132N, 11814.622W

        lat1 = aprs_to_decimal('3746.494', 'N')
        lon1 = aprs_to_decimal('12225.164', 'W')

        lat2 = aprs_to_decimal('3403.132', 'N')
        lon2 = aprs_to_decimal('11814.622', 'W')

        distance = haversine_miles(lat1, lon1, lat2, lon2)
        self.assertAlmostEqual(distance, 347.6, places=1)

    def test_ny_to_chicago(self):
        # New York: 4042.768N, 07400.360W
        # Chicago:  4152.686N, 08737.788W

        lat1 = aprs_to_decimal('4042.768', 'N')
        lon1 = aprs_to_decimal('07400.360', 'W')

        lat2 = aprs_to_decimal('4152.686', 'N')
        lon2 = aprs_to_decimal('08737.788', 'W')

        distance = haversine_miles(lat1, lon1, lat2, lon2)
        self.assertAlmostEqual(distance, 712.5, places=1)

    def test_chs_to_atl(self):
        # Charleston: 3227.590N, 07955.866W
        # Atlanta:    3344.940N, 08423.280W

        lat1 = aprs_to_decimal('3227.590', 'N')
        lon1 = aprs_to_decimal('07955.866', 'W')

        lat2 = aprs_to_decimal('3344.940', 'N')
        lon2 = aprs_to_decimal('08423.280', 'W')

        distance = haversine_miles(lat1, lon1, lat2, lon2)
        self.assertAlmostEqual(distance, 260.0, places=1)

if __name__ == '__main__':
    unittest.main()
