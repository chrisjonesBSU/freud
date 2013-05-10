from freud import locality, trajectory, localqi
import numpy as np
import numpy.testing as npt
import unittest

def FCC256():
    fcc256 = np.array([[-5.41529235,-4.91528249,-6.39377763],
    return fcc256;

class TestLocalQi(unittest.TestCase):
    def test_q6fcc(self):
        rcut = 3.7;
        testpoints = FCC256();
        box = trajectory.Box(17.661,17.661,17.661); 
        
        localq4 = localqi.LocalQi(box, rcut, 4);
        localq4.compute(testpoints);
        q4vals = localq4.getQli();
        meanq4 = np.mean(q4vals);
        
        localq6 = localqi.LocalQi(box, rcut, 6);
        localq6.compute(testpoints);
        q6vals = localq6.getQli();
        meanq6 = np.mean(q6vals);
        
        localq8 = localqi.LocalQi(box, rcut, 8);
        localq8.compute(testpoints);
        q8vals = localq8.getQli();
        meanq8 = np.mean(q8vals);
        
        localq10 = localqi.LocalQi(box, rcut, 10);
        localq10.compute(testpoints);
        q10vals = localq10.getQli();
        meanq10 = np.mean(q10vals);
        
        npt.assert_almost_equal(meanq4, 0.19, decimal=2, err_msg="Q4fail")
        npt.assert_almost_equal(meanq6, 0.56, decimal=2, err_msg="Q6fail")
        npt.assert_almost_equal(meanq8, 0.38, decimal=2, err_msg="Q8fail")
        npt.assert_almost_equal(meanq10, 0.09, decimal=2, err_msg="Q10fail")

        
        #In Steinhardt 1983 (DOI: 10.1103/PhysRevB.28.784)  Fig2 these q are ~0.2, 0.56, 0.4, (hard to read -tiny).
        
        

if __name__ == '__main__':
    unittest.main()
        
        