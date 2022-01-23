#include <ppltasks.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

#define W 600
#define H 480

struct SegmentIntersection
{
    sf::Vector2f pos;
    bool isIntersecting;
};

struct Line
{
    sf::VertexArray line;
    float angle;
};

bool operator < (const Line& left, Line& right)
{
    return left.angle < right.angle;
}


typedef std::vector<sf::ConvexShape> ConvexShapeVec;
typedef std::vector<Line> LineVec;

Line CreateLine(sf::Vector2f, sf::Vector2f);
std::shared_ptr<LineVec> LineGenerator(sf::Vector2i, std::shared_ptr<ConvexShapeVec>);
void extraLineGenerator(std::shared_ptr<LineVec>);
std::shared_ptr<ConvexShapeVec> ShapeGenerator();

void RayTracing(std::shared_ptr<ConvexShapeVec>, std::shared_ptr<LineVec>);
SegmentIntersection SegmentIntersectionDetection(const sf::ConvexShape&, Line&);

std::shared_ptr<ConvexShapeVec> CreatePolygon(std::shared_ptr<LineVec>);

sf::RenderWindow* w;

int main()
{
    sf::RenderWindow window(sf::VideoMode(W, H), "Ray Tracing Demo");
    w = &window;

    auto shapes = ShapeGenerator();

    while (window.isOpen())
    {
        auto lines = LineGenerator(sf::Mouse::getPosition(window), shapes);
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    
        window.clear();

        RayTracing(shapes, lines);

        auto polygons = CreatePolygon(lines);
        
        for (const auto& shape : *shapes)
        {
            window.draw(shape);
        }
        for (const auto& line : *lines)
        {
            window.draw(line.line);
        }
        for(const auto& shape : *polygons)
        {
            window.draw(shape); 
        }

        window.display();
    }

    return 0;
}

std::shared_ptr<std::vector<sf::ConvexShape>> ShapeGenerator()
{
    std::shared_ptr<std::vector<sf::ConvexShape>> shapes = std::make_shared<std::vector<sf::ConvexShape>>();

    std::ifstream shapeFile("./shapes.txt");

    while(shapeFile.peek() != std::char_traits<char>::eof())
    {
        sf::ConvexShape shape;
        
        int vertices;
        shapeFile >> vertices;

        shape.setPointCount(vertices);
        shape.setFillColor(sf::Color(.3f * 255, .3f * 255, .3f * 255));

        for(int i = 0; i < vertices; i++)
        {
            float x, y;
            shapeFile >> x >> y;

            shape.setPoint(i, sf::Vector2f(x, y)); 
        }
        
        shapes->push_back(shape);
    }

    
    // sf::ConvexShape shape1;
    // shape1.setPointCount(3);
    // shape1.setPoint(0, sf::Vector2f(100.f, 100.f));
    // shape1.setPoint(1, sf::Vector2f(150.f, 150.f));
    // shape1.setPoint(2, sf::Vector2f(60.f, 190.f));
    // shape1.setFillColor(sf::Color::Black);
    // shape1.setOutlineColor(sf::Color::Black);
    //
    // sf::ConvexShape shape2;
    // shape2.setPointCount(4);
    // shape2.setPoint(0, sf::Vector2f(0.f, 0.f) * 50.f + sf::Vector2f(320.f, 200.f));
    // shape2.setPoint(1, sf::Vector2f(0.f, 1.f) * 50.f + sf::Vector2f(350.f, 250.f));
    // shape2.setPoint(2, sf::Vector2f(1.f, 1.f) * 50.f + sf::Vector2f(450.f, 250.f));
    // shape2.setPoint(3, sf::Vector2f(1.f, 0.f) * 50.f + sf::Vector2f(450.f, 200.f));
    // shape2.setFillColor(sf::Color::Black);
    // shape2.setOutlineColor(sf::Color::Black);
    //
    // sf::ConvexShape shape3;
    // shape3.setPointCount(4);
    // shape3.setPoint(0, sf::Vector2f(0.f, 0.f) * 50.f + sf::Vector2f(320.f, 400.f));
    // shape3.setPoint(1, sf::Vector2f(0.f, 1.f) * 50.f + sf::Vector2f(350.f, 450.f));
    // shape3.setPoint(2, sf::Vector2f(1.f, 1.f) * 50.f + sf::Vector2f(450.f, 450.f));
    // shape3.setPoint(3, sf::Vector2f(1.f, 0.f) * 50.f + sf::Vector2f(450.f, 400.f));
    // shape3.setFillColor(sf::Color::Black);
    // shape3.setOutlineColor(sf::Color::Green);
    //
    // sf::ConvexShape shape4;
    // shape4.setPointCount(3);
    // shape4.setPoint(0, sf::Vector2f(120.f, 100.f) + sf::Vector2f(0.f, 200.f));
    // shape4.setPoint(1, sf::Vector2f(130.f, 150.f) + sf::Vector2f(0.f, 200.f));
    // shape4.setPoint(2, sf::Vector2f(60.f, 190.f) + sf::Vector2f(0.f, 200.f));
    // shape4.setFillColor(sf::Color::Black);
    // shape4.setOutlineColor(sf::Color::Black);
    //
    //
    // sf::ConvexShape shape5;
    // shape4.setPointCount(3);
    // shape4.setPoint(0, sf::Vector2f(120.f, 100.f) + sf::Vector2f(0.f, 200.f));
    // shape4.setPoint(1, sf::Vector2f(130.f, 150.f) + sf::Vector2f(0.f, 200.f));
    // shape4.setPoint(2, sf::Vector2f(60.f, 190.f) + sf::Vector2f(0.f, 200.f));
    // shape4.setFillColor(sf::Color::Black);
    // shape4.setOutlineColor(sf::Color::Black);
    // shapes->push_back(shape1);
    // shapes->push_back(shape2);
    // shapes->push_back(shape3);
    // shapes->push_back(shape4);

    return shapes;
}

Line CreateLine(sf::Vector2f start, sf::Vector2f end)
{
    sf::VertexArray line(sf::Lines, 2);
    line[0].position = sf::Vector2f(start.x, start.y);
    line[1].position = sf::Vector2f(end.x, end.y);
    
    line[0].color = sf::Color::White;
    line[1].color = sf::Color::White;

    const sf::Vector2f ray = line[1].position - line[0].position;
    const float angle = std::atan2f(ray.y, ray.x);

    return { line, angle };
}

std::shared_ptr<LineVec> LineGenerator(sf::Vector2i MousePos, std::shared_ptr<ConvexShapeVec> shapes)
{
    std::shared_ptr<LineVec> lines = std::make_shared<LineVec>();

    for(const auto& shape : *shapes)
        for(int i = 0; i < shape.getPointCount(); i++)
            lines->push_back(CreateLine(sf::Vector2f(MousePos.x, MousePos.y), shape.getPoint(i)));

    extraLineGenerator(lines);

    lines->push_back(CreateLine(sf::Vector2f(MousePos.x, MousePos.y), sf::Vector2f(0.f, 0.f)));
    lines->push_back(CreateLine(sf::Vector2f(MousePos.x, MousePos.y), sf::Vector2f(W, 0.f)));
    lines->push_back(CreateLine(sf::Vector2f(MousePos.x, MousePos.y), sf::Vector2f(0.f, H)));
    lines->push_back(CreateLine(sf::Vector2f(MousePos.x, MousePos.y), sf::Vector2f(W, H)));

    std::sort(lines->begin(), lines->end());

    return lines;
}

void extraLineGenerator(std::shared_ptr<LineVec> lines)
{
    LineVec extraLines;
    for(const auto& lineStruct : *lines)
    {
        sf::VertexArray line = lineStruct.line;
        
        const sf::Vector2f ray = line[1].position - line[0].position;
        const float theta = std::atan2f(ray.y, ray.x);

        const float oneMoreTheta = theta + 3.14 / 180 / 2;
        Line oneMoreLine = CreateLine(
            line[0].position,
            sf::Vector2f(cos(oneMoreTheta), sin(oneMoreTheta)) * 3000.f + line[0].position);
        
        const float oneLessTheta = theta - 3.14 / 180 / 2;
        Line oneLessLine = CreateLine(
            line[0].position,
            sf::Vector2f(cos(oneLessTheta), sin(oneLessTheta)) * 3000.f + line[0].position);

        extraLines.push_back(oneMoreLine);
        extraLines.push_back(oneLessLine);
    }

    for(const auto& line: extraLines)
        lines->push_back(line);
}

void RayTracing(std::shared_ptr<ConvexShapeVec> shapes, std::shared_ptr<LineVec> lines)
{
     for(auto& line : *lines)
     {
         for(const auto& shape : *shapes)
         {
             const SegmentIntersection result = SegmentIntersectionDetection(shape, line);

             if (result.isIntersecting)
             {
                 line.line[1].position = result.pos;
             }
         }
     }
}

void printvec(std::string name, sf::Vector2f v)
{
    std::cout << name << ": (" << v.x << ", " << v.y << ")" << std::endl;
}

SegmentIntersection SegmentIntersectionDetection(const sf::ConvexShape& shape, Line& lineStruct)
{
    const sf::VertexArray line = lineStruct.line;
    SegmentIntersection current = { sf::Vector2f(0.f, 0.f), false};
    
    const sf::Vector2f a = line[0].position; // mouse
    const sf::Vector2f b = line[1].position; // vertices
    
    for (unsigned int i = 0; i < shape.getPointCount(); i++)
    {
        const sf::Vector2f c = shape.getPoint(i);
        const sf::Vector2f d = shape.getPoint((i + 1) % shape.getPointCount());
    
        const sf::Vector2f r = b - a;
        const sf::Vector2f s = d - c;

        const sf::Vector2f cma = c - a;
        const float rxs = r.x * s.y - s.x * r.y;

        const float t = (cma.x * s.y - cma.y * s.x) / rxs;
        const float u = (cma.x * r.y - cma.y * r.x) / rxs;

        if((t >= 0.f && t <= 1.f) && (u >= 0.f && u <= 1.f))
        {
            const sf::Vector2f currentR = current.pos - a;
            const sf::Vector2f newR = t * r;

            const sf::Vector2f newIntersect = a + newR;
            
            const float currentLenSq = pow(currentR.x + currentR.y, 2);
            const float newLenSq = pow(newR.x + newR.y, 2);
            
            if(!current.isIntersecting || newLenSq < currentLenSq)
            {
                current.pos = newIntersect;
                current.isIntersecting = true;
            }
        }
    }

    return current;
}

std::shared_ptr<ConvexShapeVec> CreatePolygon(std::shared_ptr<LineVec> lines)
{
    std::shared_ptr<ConvexShapeVec> shapes = std::make_shared<ConvexShapeVec>();
    for(int i = 0; i < lines->size(); i++)
    {
        const sf::VertexArray line = lines->at(i).line;
        const sf::VertexArray nextLine = lines->at((i + 1) % lines->size()).line;

        sf::ConvexShape polygon;
        polygon.setPointCount(3);

        polygon.setPoint(0, line[0].position);
        polygon.setPoint(1, line[1].position);
        polygon.setPoint(2, nextLine[1].position);

        polygon.setFillColor(sf::Color(.8f * 255, .8f * 255, .8f * 255));

        shapes->push_back(polygon);
    }

    return shapes;
}