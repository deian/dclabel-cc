{- |
  
   Generates several test files in /tmp/gen_test/
   that tests the C++ implementation of DCLabels

   This code is ugly.
   The code it generates is ugly.
   But I hate C++ more.

-}

{-# LANGUAGE ScopedTypeVariables #-}
import Control.Monad
import qualified Control.Exception as E
import Data.Monoid
import Data.List (intercalate)
import qualified Data.ByteString.Char8 as S8
import qualified LIO as D
import qualified LIO.DCLabel as D
import LIO.DCLabel.Instances
import Test.QuickCheck
import Test.QuickCheck.Instances
import System.Directory

genDisj name d = unlines [
    "{"
  , "Principal ps[] = {" ++ (intercalate "," ps) ++ "};"
  , name ++ " = Clause(ps,"++len++");"
  , "}" ]
  where len = show $ length d
        ps = map (show . S8.unpack . D.principalName) d

genComponent name c | D.isTrue  c = name ++ " = Component::dcTrue();" 
                    | D.isFalse c = name ++ " = Component::dcFalse();" 
                    | otherwise   = unlines [
                      "{"
                    , "std::set<Clause> "++sname++";"
                    , unlines $ zipWith genDisj' ns cs
                    , name ++ " = Component::dcFormula("++sname++");"
                    , "}" ]
                    where cs = D.toList c
                          ns = map (\i -> "c"++ show i) [1..(length cs)]
                          sname = "_set_"++name
                          genDisj' n c = unlines [
                              "{"
                            , "Clause "++ n ++ ";"
                            , genDisj n c
                            , sname++".insert("++n++");"
                            , "}"]

genLabel name l | l == D.bottom = name ++" = DCLabel::dcBottom();"
                | l == D.top    = name ++" = DCLabel::dcTop();"
                | l == D.dcPub  = name ++" = DCLabel::dcTop();"
                |otherwise      = unlines [
                  "{"
                , "Component s, i;"
                , genComponent "s" (D.dcSecrecy l)
                , genComponent "i" (D.dcIntegrity l)
                , name ++ " = DCLabel(s, i);"
                , "}" ]



canFlowTo lr l1 l2 = unlines [
   "{"
 , "DCLabel l1, l2;"
 , genLabel "l1" l1 
 , genLabel "l2" l2
 , lr ++ " = l1.canFlowTo(l2);"
 , "}" ]

join lr l1 l2 = unlines [
   "{"
 , "DCLabel l1, l2;"
 , genLabel "l1" l1 
 , genLabel "l2" l2
 , lr ++ " = lub(l1, l2);"
 , "}" ]

meet lr l1 l2 = unlines [
   "{"
 , "DCLabel l1, l2;"
 , genLabel "l1" l1 
 , genLabel "l2" l2
 , lr ++ " = glb(l1, l2);"
 , "}" ]

--
--
--

data CProp = CProp { propCall :: String
                   , propDef  :: String 
                   } deriving (Show)
mkProp name body = CProp {
    propCall = fname ++ ";"
  , propDef  = "bool "++fname++" {\n" ++ body ++ "\n}//---" ++ fname
  } 
  where fname = "prop_"++name++"()"

prop_dcReduce_no_change :: String -> D.DCLabel -> CProp
prop_dcReduce_no_change name l1 = mkProp name $ unlines [
    "DCLabel l1, l2;"
  , genLabel "l1" l1
  , "l2 = DCLabel(l1);"
  , "l1.dcReduce();"
  , "return l1.canFlowTo(l2) && l2.canFlowTo(l1);"
  ]

prop_dcReduce_idem :: String -> D.DCLabel -> CProp
prop_dcReduce_idem name l1 = mkProp name $ unlines [
    "DCLabel l1, l2;"
  , genLabel "l1" l1
  , "l1.dcReduce();"
  , "l2 = DCLabel(l1);"
  , "l1.dcReduce();"
  , "return l1 == l2;"
  ]

prop_dc_top :: String -> D.DCLabel -> CProp
prop_dc_top name l1 = mkProp name $ unlines [
    "DCLabel l1;"
  , genLabel "l1" l1
  , "return l1.canFlowTo(DCLabel::dcTop());" ]

prop_dc_bot :: String -> D.DCLabel -> CProp
prop_dc_bot name l1 = mkProp name $ unlines [
    "DCLabel l1, bot=DCLabel::dcBottom();"
  , genLabel "l1" l1
  , "return bot.canFlowTo(l1);"]

prop_dc_join :: String -> D.DCLabel -> D.DCLabel -> CProp
prop_dc_join name l1 l2  = mkProp name $ unlines [
    "DCLabel l1, l2, l3;"
  , genLabel "l1" l1
  , genLabel "l2" l2
  , "l3 = DCLabel::lub(l1,l2);"
  , "return l1.canFlowTo(l3) && l2.canFlowTo(l3);" ]

prop_dc_join_lub :: String -> D.DCLabel -> D.DCLabel -> [D.DCLabel] -> CProp
prop_dc_join_lub name l1 l2 l3s = mkProp name $ unlines $ [
    "DCLabel l1, l2;"
  , genLabel "l1" l1
  , genLabel "l2" l2
  , "bool rc = true;"] ++
  map (\l3 -> unlines [ "{"
                      , "DCLabel l3;"
                      , genLabel "l3" l3
                      , "if(l1.canFlowTo(l3) && l2.canFlowTo(l3)) {"
                      , "DCLabel l12 = DCLabel::lub(l1,l2);"
                      , "rc &= l12.canFlowTo(l3);"
                      , "}"
                      , "}"]) l3s ++
  [ "return rc;" ]

prop_dc_join_glb :: String -> D.DCLabel -> D.DCLabel -> [D.DCLabel] -> CProp
prop_dc_join_glb name l1 l2 l3s = mkProp name $ unlines $ [
    "DCLabel l1, l2;"
  , genLabel "l1" l1
  , genLabel "l2" l2
  , "bool rc = true;"] ++
  map (\l3 -> unlines [ "{"
                      , "DCLabel l3;"
                      , genLabel "l3" l3
                      , "if(l3.canFlowTo(l1) && l3.canFlowTo(l2)) {"
                      , "DCLabel l12 = DCLabel::glb(l1,l2);"
                      , "rc &= l3.canFlowTo(l12);"
                      , "}"
                      , "}"]) l3s ++
  [ "return rc;" ]


prop_dc_meet :: String -> D.DCLabel -> D.DCLabel -> CProp
prop_dc_meet name l1 l2  = mkProp name $ unlines [
    "DCLabel l1, l2, l3;"
  , genLabel "l1" l1
  , genLabel "l2" l2
  , "l3 = DCLabel::glb(l1,l2);"
  , "return l3.canFlowTo(l1) && l3.canFlowTo(l2);" ]

prop_dc_porder :: String -> D.DCLabel -> D.DCLabel -> CProp
prop_dc_porder name l1 l2  = mkProp name $ unlines [
    "DCLabel l1, l2;"
  , genLabel "l1" l1
  , genLabel "l2" l2
  , "bool ge = l1.canFlowTo(l2);"
  , "bool le = l2.canFlowTo(l1);"
  , "bool eq = l1 == l2;"
  , "return (eq && ge && le) ||  // == \n\
            \((not eq) && (ge || le) && (ge != le)) || // < or >\n\
            \(not (eq || ge || le)); // incomparable"]


--
--
--

gen_Unary :: Int -> IO [CProp]
gen_Unary n = do
  ls <- head `liftM` sample' (vectorOf n (arbitrary :: Gen D.DCLabel))
  let ps1 = zipWith (\i l -> 
              prop_dcReduce_no_change ("dcReduce_no_change"++ show i) l)
      ps2 = zipWith (\i l -> 
              prop_dcReduce_idem ("dcReduce_idem"++ show i) l)
      ps3 = zipWith (\i l -> 
              prop_dc_top ("dc_top"++ show i) l)
      ps4 = zipWith (\i l -> 
              prop_dc_bot ("dc_bot"++ show i) l)
  return $ ps1 [1..] ls
        ++ ps2 [1..] ls
        ++ ps3 [1..] ls
        ++ ps4 [1..] ls

gen_Binary :: Int -> IO [CProp]
gen_Binary n = do
  ls1 <- head `liftM` sample' (vectorOf n (arbitrary :: Gen D.DCLabel))
  ls2 <- head `liftM` sample' (vectorOf n (arbitrary :: Gen D.DCLabel))
  let ps1 = zipWith3 (\i l1 l2 -> 
              prop_dc_join ("dc_join"++ show i) l1 l2)
      ps2 = zipWith3 (\i l1 l2 -> 
              prop_dc_meet ("dc_meet"++ show i) l1 l2)
      ps3 = zipWith3 (\i l1 l2 -> 
              prop_dc_porder ("dc_porder"++ show i) l1 l2)
  return $ ps1 [1..] ls1 ls2
        ++ ps2 [1..] ls1 ls2
        ++ ps3 [1..] ls1 ls2
  
gen_Binary_all :: Int -> IO [CProp]
gen_Binary_all n = do
  ls1 <- head `liftM` sample' (vectorOf n (arbitrary :: Gen D.DCLabel))
  ls2 <- head `liftM` sample' (vectorOf n (arbitrary :: Gen D.DCLabel))
  ls3 <- head `liftM` sample' (vectorOf n (arbitrary :: Gen D.DCLabel))
  let ps1 = zipWith3 (\i l1 l2 -> 
              prop_dc_join_lub ("dc_join_lub"++ show i) l1 l2 ls3)
      ps2 = zipWith3 (\i l1 l2 -> 
              prop_dc_join_glb ("dc_join_glb"++ show i) l1 l2 ls3)
  return $ ps1 [1..] ls1 ls2
        ++ ps2 [1..] ls1 ls2


genTests :: [CProp] -> String
genTests ps = unlines $ [
    "#include \"dclabel.h\""
  , "#include <iostream>"
  , "#include <set>"
  , ""
  ] ++  map propDef ps ++ 
  ["int main() {"
  , "bool rc;"
  ]  ++
  map (\p -> unlines [
      "std::cout << \"Running "++ propCall p++"...\";"
    , "rc = " ++ propCall p ++ ";"
    , "if(rc) {"
    , "  std::cout << \"OK!\" << std::endl;"
    , "} else { "
    , "  std::cout << \"FAIL!\" << std::endl;"
    , "  return -1;"
    , "}"
    ]) ps
  ++ ["return 0;"
     ,"}"]

main = do
  let ts = [ gen_Unary n
           , gen_Binary n
           , gen_Binary_all n]
  putStrLn $ "Each test runs "++ show n ++ " cases."
  forM_ (zip [1..] ts) $ \(i, tact) -> do
    let tname = "/tmp/gen_tests/test"++ show i
    createDirectory "/tmp/gen_tests" `catchIO` return ()
    prop <- tact
    putStr $ "Writing "++tname++".cc ..."
    writeFile (tname++".cc") $ genTests prop
    putStrLn "DONE."
  where n = 100
        catchIO a h = E.catch a (\(_ :: E.IOException) -> h)
