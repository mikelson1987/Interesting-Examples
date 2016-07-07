# Interesting-Examples
-------------------------------------------------------------
Использование move-семантики
vector<ifstream> produce()
{
  vector<ifstream> ans;
  for (int i = 0;  i < 10; ++i) {
    ans.emplace_back(name(i));
  } 
  return ans;
}
 
void consumer()
{
  vector<ifstream> files = produce();
  for (ifstream& f: files) {
    read(f);
  }
} // close all files

-------------------------------------------------
Выгрузка объектов при "ненужности"

std::shared_ptr<const Widget> fastLoadWidget (Widget ID id);
{
static std::unordered_map<WidgetID, std::weak_ptr<const Widget>> cache ;
auto objPtr           // obj Ptr является std: : shared_ptr
= cache[id].lock();   // для кешированного объекта и
if (!objPtr) {
  objPtr = loadWidget(id);
  cache[id] = objPtr;
}
return objPtr;
}


//Оптимизация запроса Sql запроса

UPDATE TableName
SET A = @VALUE
WHERE
      B = 'YOUR CONDITION'
            AND A <> @VALUE – VALIDATION
